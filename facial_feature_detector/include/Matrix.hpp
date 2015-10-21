/*
 * Mat.h
 *
 *  Created on: 2013. 11. 11.
 *      Author: minu
 */

#ifndef __DAUMCV_MAT_HPP__
#define __DAUMCV_MAT_HPP__

#include <iostream>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <typeinfo>
#include <limits>
#include <cmath>
using namespace std;

#include "Core.hpp"

namespace daum {

typedef unsigned char uchar;

enum MatType
{
	kZeros=0,
	kOnes,
	kIdentity
};

template<typename _Tp>

class Matrix_ {
public:
	Matrix_(int _rows=1, int _cols=1, MatType type=kZeros);
	Matrix_(int _rows, int _cols, const _Tp* data);
	Matrix_(const Matrix_& mat);
	template<typename _Tp2> Matrix_(const Matrix_<_Tp2>& mat);
	~Matrix_();

	Matrix_& operator = (const Matrix_& mat);

	template<typename _Tp2> const _Tp2 at(int r, int c=0) const;
	template<typename _Tp2> _Tp2& at(int r, int c);
	const _Tp at(int r, int c=0) const;
	_Tp& at(int r, int c);
    //const _Tp* operator [](int i) const;
    //_Tp& operator[](int i);

	Matrix_ t();
	Matrix_ inv();

	_Tp* ptr(int i=0);
	const _Tp* ptr(int i=0) const;

	_Tp* ptr(int i, int j);
	const _Tp* ptr(int i, int j) const;

	Size size() const;

	static Matrix_ zeros(int _rows, int _cols);
	static Matrix_ zeros(Size size);
	static Matrix_ ones(int _rows, int _cols);
	static Matrix_ ones(Size size);
	static Matrix_ eye(int _rows, int _cols);
	static Matrix_ eye(Size size);

	int rows, cols;

private:

	int size_;

	_Tp* data_;
	_Tp** row_data_;

	void InitData(MatType type=kZeros);
	void Alloc();
	void Delete();
};

template<typename _Tp> inline
Matrix_<_Tp>::Matrix_(int _rows, int _cols, MatType type) :
	rows(_rows), cols(_cols), size_(rows * cols), data_(NULL), row_data_(NULL)
{
	Alloc();
	InitData(type);
}

template<typename _Tp> inline
Matrix_<_Tp>::Matrix_(int _rows, int _cols, const _Tp* data) :
	rows(_rows), cols(_cols), size_(rows * cols), data_(NULL), row_data_(NULL)
{
	Alloc();
	memcpy(data_, data, size_ * sizeof(_Tp));
}

template<typename _Tp> inline
Matrix_<_Tp>::Matrix_(const Matrix_<_Tp> & mat) :
	rows(mat.rows), cols(mat.cols), size_(rows * cols), data_(NULL), row_data_(NULL)
{
	Alloc();
	memcpy(data_, mat.ptr(), size_ * sizeof(_Tp));
}

template<typename _Tp> template<typename _Tp2> inline
Matrix_<_Tp>::Matrix_(const Matrix_<_Tp2>& mat) :
	rows(mat.rows), cols(mat.cols), size_(rows * cols), data_(NULL), row_data_(NULL)
{
	Alloc();

	const _Tp2* src = mat.ptr();
	_Tp* dst = data_;
	for ( int i=0 ; i<size_ ; ++i )
	{
		*dst = static_cast<_Tp>(*src);
		++src;
		++dst;
	}
}


template<typename _Tp> inline
void Matrix_<_Tp>::InitData(MatType type)
{
	if ( type == kZeros || type == kOnes) {
		_Tp value = static_cast<_Tp>(type);
		_Tp* src = data_;
		for ( int i=0 ; i<cols ; ++i ) src[i] = value;
		src += cols;

		for ( int i=1 ; i<rows ; ++i )
		{
			memcpy(src, data_, sizeof(_Tp) * cols);
			src += cols;
		}
	}
	else  // kIdentity
	{
		assert ( type == kIdentity);
		assert ( rows == cols );

		InitData(kZeros);
		_Tp* src = data_;
		for ( int i=0 ; i<rows ; ++i )
		{
			*src = static_cast<_Tp>(1);
			src += cols + 1;
		}
	}
}


template<typename _Tp> template<typename _Tp2> inline
const _Tp2 Matrix_<_Tp>::at(int r, int c) const
{
	assert ( r>=0 && c>=0 && r<rows && c<cols );
	return static_cast<_Tp2>(*((_Tp2*)row_data_[r] + c));
}

template<typename _Tp> template<typename _Tp2> inline
_Tp2& Matrix_<_Tp>::at(int r, int c)
{
	assert ( r>=0 && c>=0 && r<rows && c<cols );
	return *((_Tp2*)row_data_[r] + c);
}


template<typename _Tp> inline
const _Tp Matrix_<_Tp>::at(int r, int c) const
{
	assert ( r>=0 && c>=0 && r<rows && c<cols );
	return *(row_data_[r] + c);
}

template<typename _Tp> inline
_Tp& Matrix_<_Tp>::at(int r, int c)
{
	assert ( r>=0 && c>=0 && r<rows && c<cols );
	return *(row_data_[r] + c);
}


/*
template<typename _Tp> inline
const _Tp* Mat<_Tp>::operator [](int i) const {
	return data_ + i * cols_;
}

template<typename _Tp> inline
_Tp& Mat<_Tp>::operator[](int i) {
	return *(data_ + i * cols_);
}
*/

template<typename _Tp> inline
Matrix_<_Tp>& Matrix_<_Tp>::operator = ( const Matrix_& mat) {
	assert( typeid(*mat.ptr()).name() == typeid(*this->ptr()).name() );
	if ( mat.rows != this->rows || mat.rows != this->cols )
	{
		Delete();

		rows = mat.rows;
		cols = mat.cols;
		size_ = mat.rows * mat.cols;
		Alloc();
	}
	memcpy(data_, mat.ptr(), sizeof(_Tp) * size_);
	return *this;
}

template<typename _Tp> inline
Matrix_<_Tp> Matrix_<_Tp>::t()
{
	Matrix_<_Tp> res(cols, rows);
	_Tp* src = data_;
	for ( int i=0 ; i<rows ; ++i )
	{
		for ( int j=0 ; j<cols ; ++j )
		{
			res.at<_Tp>(j, i) = *(src++);
		}
	}
	return res;
}

template<typename _Tp> inline
Matrix_<_Tp> Matrix_<_Tp>::inv() {
	int m = rows, n = cols;
	assert(m == n);
	assert(typeid(_Tp) == typeid(float) || typeid(_Tp) == typeid(double) );

	Matrix_<_Tp> tmp(*this);

	_Tp* src = tmp.ptr();
	Matrix_<float> res(m, m, kIdentity);
	_Tp* dst = res.ptr();

	_Tp* A = src;
	_Tp* b = dst;

	// LU Decomposition
	for (int i = 0; i < m; i++) {
		int p = i;
		for (int j = i + 1; j < m; j++) {
			if (fabs(A[j * cols + i]) > fabs(A[p * cols + i]))
				p = j;
		}

		//if (abs(A[p * cols + i]) < std::numeric_limits<_Tp>::epsilon()) {
		if (fabs(A[p * cols + i]) < 1e-7) {
			// TODO: what's happen?
			return res;
		}

		if (p != i) {
			for (int j = i; j < m; j++) {
				std::swap(A[i * cols + j], A[p * cols + j]);
			}
			for (int j = 0; j < n; j++) {
				std::swap(b[i * cols + j], b[p * cols + j]);
			}
		}

		_Tp d = -1 / A[i * cols + i];

		for (int j = i + 1; j < m; j++) {
			_Tp alpha = A[j * cols + i] * d;

			for (int k = i + 1; k < m; k++) {
				A[j * cols + k] += alpha * A[i * cols + k];
			}
			for (int k = 0; k < n; k++) {
				b[j * cols + k] += alpha * b[i * cols + k];
			}
		}

		A[i * cols + i] = -d;
	}

	for (int i = m - 1; i >= 0; i--) {
		for (int j = 0; j < n; j++) {
			_Tp s = b[i * cols + j];
			for (int k = i + 1; k < m; k++)
				s -= A[i * cols + k] * b[k * cols + j];
			b[i * cols + j] = s * A[i * cols + i];
		}
	}
	return res;
}

template<typename _Tp> inline
_Tp* Matrix_<_Tp>::ptr(int i) {
	assert ( i>=0 && i<rows );
	return row_data_[i];
}
template<typename _Tp> inline
const _Tp* Matrix_<_Tp>::ptr(int i) const {
	assert ( i>=0 && i<rows );
	return row_data_[i];
}
template<typename _Tp> inline _Tp* Matrix_<_Tp>::ptr(int i, int j) {
	assert ( i>=0 && j>=0 && i<rows && j<cols );
	return row_data_[i] + j;
}
template<typename _Tp> inline const _Tp* Matrix_<_Tp>::ptr(int i, int j) const {
	assert ( i>=0 && j>=0 && i<rows && j<cols );
	return row_data_[i] + j;
}

template<typename _Tp> inline Size Matrix_<_Tp>::size() const { return Size(cols, rows); }


template<typename _Tp> inline
Matrix_<_Tp> Matrix_<_Tp>::zeros(int _rows, int _cols)
{
	return Matrix_(_rows, _cols, kZeros);
}

template<typename _Tp> inline
Matrix_<_Tp> Matrix_<_Tp>::zeros(Size size)
{
	return Matrix_(size.height, size.width, kZeros);
}

template<typename _Tp> inline
Matrix_<_Tp> Matrix_<_Tp>::ones(int _rows, int _cols)
{
	return Matrix_(_rows, _cols, kOnes);
}

template<typename _Tp> inline
Matrix_<_Tp> Matrix_<_Tp>::ones(Size size)
{
	return Matrix_(size.height, size.width, kOnes);
}

template<typename _Tp> inline
Matrix_<_Tp> Matrix_<_Tp>::eye(int _rows, int _cols)
{
	return Matrix_(_rows, _cols, kIdentity);
}

template<typename _Tp> inline
Matrix_<_Tp> Matrix_<_Tp>::eye(Size size)
{
	return Matrix_(size.height, size.width, kIdentity);
}

template<typename _Tp> inline
std::ostream& operator << (std::ostream& os, const Matrix_<_Tp>& m)
{
	// TODO:
	for ( int i=0 ; i<m.rows ; ++i )
	{
		for ( int j=0 ; j<m.cols ; ++j )
		{
			os << m.at(i, j) << ' ';
		}
		os << std::endl;
	}
    return os;
}


template<typename _Tp> static inline
Matrix_<_Tp> operator + (const Matrix_<_Tp>& a, const Matrix_<_Tp>& b)
{
	assert(a.rows == b.rows && a.cols == b.cols );

	Matrix_<_Tp> res(a);
	_Tp* ptr_a = res.ptr();
	const _Tp* ptr_b = b.ptr();
	for ( int i=0 ; i<a.rows ; ++i )
	{
		for ( int j=0 ; j<a.cols ; ++j )
		{
			ptr_a[j] += ptr_b[j];
		}
		ptr_a += res.cols;
		ptr_b += res.cols;
	}
	return res;
}

template<typename _Tp> static inline
Matrix_<_Tp> operator - (const Matrix_<_Tp>& a, const Matrix_<_Tp>& b)
{
	assert(a.rows == b.rows && a.cols == b.cols );

	Matrix_<_Tp> res(a);
	_Tp* ptr_a = res.ptr();
	const _Tp* ptr_b = b.ptr();
	for ( int i=0 ; i<a.rows ; ++i )
	{
		for ( int j=0 ; j<a.cols ; ++j )
		{
			ptr_a[j] -= ptr_b[j];
		}
		ptr_a += res.cols;
		ptr_b += res.cols;
	}
	return res;
}

template<typename _Tp> static inline
Matrix_<_Tp> operator * (const Matrix_<_Tp>& a, const Matrix_<_Tp>& b)
{
	assert(a.cols == b.rows );

	// TODO: 더 빠르게!
	int m = a.rows;
	int n = b.cols;
	int p = b.rows;
	Matrix_<_Tp> res(m, n);

	for ( int i=0 ; i<m ; ++i )
	{
		for ( int j=0 ; j<n ; ++j )
		{
			_Tp sum = static_cast<_Tp>(0.0);
			for ( int k=0 ; k<p ; ++k )
			{
				sum += a.at(i, k) * b.at(k, j);
			}
			res.at(i, j) = sum;
		}
	}
	return res;
}

template<typename _Tp> static inline
Matrix_<_Tp>& operator += (Matrix_<_Tp>& a, const Matrix_<_Tp>& b)
{
	assert(a.rows == b.rows && a.cols == b.cols );

	_Tp* ptr_a = a.ptr();
	const _Tp* ptr_b = b.ptr();
	for ( int i=0 ; i<a.rows ; ++i )
	{
		for ( int j=0 ; j<a.cols ; ++j )
		{
			ptr_a[j] += ptr_b[j];
		}
		ptr_a += a.cols;
		ptr_b += a.cols;
	}
    return a;
}

template<typename _Tp> static inline
Matrix_<_Tp>& operator -= (Matrix_<_Tp>& a, const Matrix_<_Tp>& b)
{
	assert(a.rows == b.rows && a.cols == b.cols );

	_Tp* ptr_a = a.ptr();
	const _Tp* ptr_b = b.ptr();
	for ( int i=0 ; i<a.rows ; ++i )
	{
		for ( int j=0 ; j<a.cols ; ++j )
		{
			ptr_a[j] -= ptr_b[j];
		}
		ptr_a += a.cols;
		ptr_b += a.cols;
	}
    return a;
}

template<typename _Tp>
void Matrix_<_Tp>::Alloc() {
	assert ( data_ == NULL );
	data_ = new _Tp [size_];

	assert ( row_data_ == NULL );
	row_data_ = new _Tp* [rows];

	row_data_[0] = data_;
	for ( int i=1 ; i<rows ; ++i )
	{
		row_data_[i] = row_data_[i-1] + cols;
	}
}

template<typename _Tp>
void Matrix_<_Tp>::Delete() {
	assert ( data_ != NULL );
	if ( data_ ) delete [] data_;
	data_ = NULL;

	assert ( row_data_ != NULL );
	if ( row_data_ ) delete [] row_data_;
	row_data_ = NULL;
}

template<typename _Tp>
Matrix_<_Tp>::~Matrix_() {
	Delete();
}

} /* namespace daum */

#endif /* __DAUMCV_MAT_H__ */
