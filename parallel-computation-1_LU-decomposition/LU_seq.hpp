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

	Matrix<T> L(b, N), U(N - b, b);

	for (size_t i = 0; i < N; i += b) {
		// (1)
		// Find LU decomposition of block
		const size_t rows = N - i;
		const size_t cols = b;
		// Copy matrices L and U from matrix A
		for (size_t _i = 0; _i < cols; ++_i)
			for (size_t _j = 0; _j < rows; ++_j) 
				L(_i, _j) = A(_i + i, _j + i);

		for (size_t _i = 0; _i < rows; ++_i)
			for (size_t _j = 0; _j < cols; ++_j)
				U(_i, _j) = A(_i + i + b, _j + i);
		LU_seq(L);

		// (2)
		// Solve systems L22 * U23 = A23
		// to get U23 = L22^-1 * A23
		span_set_product_inverseL_by_self(A, L, U, i);
	
		// (3)
		// A33 -= A32 * A23
		span_substract_product(A, L, U, i);
	}
}

template <typename T>
Matrix<T> product_LU(const Matrix<T>& A) {
	Matrix<T> res(A);

	for (size_t i = 1; i < A.rows(); ++i)
		for (size_t j = 0; j < A.cols(); ++j) {
			res(i, j) = 0;
			for (size_t k = 0; k <= i; ++k)
				if(k == i)
					res(i, j) += A(k, j);
				else
					res(i, j) += A(i, k) * A(k, j);
		}
	return(res);
 }

// Splits LU saved as a single matrix into separate objects
//template <typename T>
//void split_into_LU(const Matrix<T>& A, Matrix<T>& L, Matrix<T>& U) {
//	const auto MIN = std::min(A.rows(), A.cols());
//
//	// L (ROWS x ROWS)
//	for (size_t i = 0; i < A.rows(); ++i) {
//		for (size_t j = 0; j < std::min(i, MIN); ++j) L(i, j) = A(i, j);
//		if (i < MIN) L(i, i) = 1;
//		for (size_t j = i + 1; j < MIN; ++j) L(i, j) = 0;
//	}
//
//	// U (ROWS x COLS)
//	for (size_t i = 0; i < MIN; ++i) {
//		for (size_t j = 0; j < i; ++j) U(i, j) = 0;
//		for (size_t j = i; j < A.cols(); ++j) U(i, j) = A(i, j);
//	}
//}