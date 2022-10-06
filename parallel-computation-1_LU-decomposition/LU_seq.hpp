#pragma once

#include "matrix.hpp"
#include "matric_span.hpp"


// LU decomposition
// - Sequential
// - No pivoting
// - Time complexity O(2/3 N^3)
template <typename T>
void LU_seq(Matrix<T> &A) {
	const size_t ROWS = A.rows();
	const size_t COLS = A.cols();

	for (size_t i = 0; i < std::min(ROWS - 1, COLS); ++i) {
		// (1)
		const T inverseAii = T(1) / A(i, i);

		for (size_t j = i + 1; j < ROWS; ++j)
			A(j, i) *= inverseAii;

		// (2)
		for (size_t j = i + 1; j < ROWS; ++j)
			for (size_t k = i + 1; k < COLS; ++k)
				A(j, k) -= A(j, i) * A(i, k);
	}
}

// Block LU decomposition
// - Sequential
// - No pivoting
// - Time complexity O(?)
template <typename T>
void LU_seq_block(Matrix<T>& A, const size_t b) {
	// ROWS = COLS like in the literature
	const size_t N = A.rows();

	Matrix<T> A_23(b, N - b);
	Matrix<T> A_22_32(N, b);

	for (size_t i = 0; i < N; i += b) {
		// (1)
		// Find LU decomposition of block (A22 & A32)
		const size_t rows1 = N - i;
		const size_t cols1 = b;

		A_22_32.downsize(N - i, b);
		span_copy(A,       i, i, rows1, cols1, 
			      A_22_32, 0, 0);

		span_LU_decomposition(A_22_32, 0, 0, rows1, cols1);

		span_copy(A_22_32, 0, 0, rows1, cols1,
			      A,       i, i);

		if (!(i < N - b)) break;

		// (2)
		// Solve systems L22 * U23 = A23
		// to get U23 = L22^-1 * A23
		A_23.downsize(b, N - b - i);
		span_copy(A,    i, i + b, b, N - b - i,
			      A_23, 0, 0);

		span_set_product_inverseL_by_self(A_22_32, 0, 0, b, b,
										  A_23,    0, 0, b, N - b - i);

		span_copy(A_23, 0, 0, b, N - b - i, A, i, i + b);
		
		// (3)
		span_substract_product(A_22_32, b,     0,    N - b - i, b,
							   A_23,    0,     0,    b,         N - b - i, 
			                   A,       i + b, i + b);
	}
}

// Splits LU saved as a single matrix into separate objects
template <typename T>
void split_into_LU(const Matrix<T>& A, Matrix<T>& L, Matrix<T>& U) {
	const auto MIN = std::min(A.rows(), A.cols());

	// L (ROWS x ROWS)
	for (size_t i = 0; i < A.rows(); ++i) {
		for (size_t j = 0; j < std::min(i, MIN); ++j) L(i, j) = A(i, j);
		if (i < MIN) L(i, i) = 1;
		for (size_t j = i + 1; j < MIN; ++j) L(i, j) = 0;
	}

	// U (ROWS x COLS)
	for (size_t i = 0; i < MIN; ++i) {
		for (size_t j = 0; j < i; ++j) U(i, j) = 0;
		for (size_t j = i; j < A.cols(); ++j) U(i, j) = A(i, j);
	}
}

template <typename T>
Matrix<T> product_LU(const Matrix<T>& A) {
	Matrix<T> res(A);

	for (size_t i = 1; i < A.rows(); ++i)
		for (size_t j = 0; j < A.cols(); ++j) 
		{
			res(i, j) = 0;
			for (size_t k = 0; k <= std::min(j, i); ++k)
				res(i, j) += k == i ? A(i, j) : A(i, k) * A(k, j);
		}
	return(res);
}