/*
 * Point.hpp
 *
 *  Created on: Nov 13, 2013
 *      Author: minu
 */

#ifndef __DAUM__CORE_HPP__
#define __DAUM__CORE_HPP__

namespace daum {

template<typename _Tp> class Point_;
template<typename _Tp> class Size_;
template<typename _Tp> class Rect_;

typedef Point_<int> Point2i;
typedef Point2i Point;
typedef Size_<int> Size2i;
typedef Size2i Size;
typedef Rect_<int> Rect;
typedef Point_<float> Point2f;
typedef Point_<double> Point2d;
typedef Size_<float> Size2f;

template<typename _Tp>
class Point_ {
public:
	Point_();
	Point_(_Tp a, _Tp b);
	Point_(const Point_& p);

	Point_& operator =(const Point_& pt);
	template<typename _Tp2> operator Point_<_Tp2>() const;

    //! dot product
    _Tp dot(const Point_& pt) const;
    //! dot product computed in double-precision arithmetics
    double ddot(const Point_& pt) const;
    //! cross-product
    double cross(const Point_& pt) const;
    //! checks whether the point is inside the specified rectangle
    bool inside(const Rect_<_Tp>& r) const;

	_Tp x, y;
};


template<typename _Tp> class Size_
{
public:
    typedef _Tp value_type;

    //! various constructors
    Size_();
    Size_(_Tp _width, _Tp _height);
    Size_(const Size_& sz);
    Size_(const Point_<_Tp>& pt);

    Size_& operator = (const Size_& sz);
    //! the area (width*height)
    _Tp area() const;

    //! conversion of another data type.
    template<typename _Tp2> operator Size_<_Tp2>() const;

    _Tp width, height; // the width and the height
};

template<typename _Tp> class Rect_
{
public:
    typedef _Tp value_type;

    //! various constructors
    Rect_();
    Rect_(_Tp _x, _Tp _y, _Tp _width, _Tp _height);
    Rect_(const Rect_& r);
    Rect_(const Point_<_Tp>& org, const Size_<_Tp>& sz);
    Rect_(const Point_<_Tp>& pt1, const Point_<_Tp>& pt2);

    Rect_& operator = ( const Rect_& r );
    //! the top-left corner
    Point_<_Tp> tl() const;
    //! the bottom-right corner
    Point_<_Tp> br() const;

    //! size (width, height) of the rectangle
    Size_<_Tp> size() const;
    //! area (width*height) of the rectangle
    _Tp area() const;

    //! conversion to another data type
    template<typename _Tp2> operator Rect_<_Tp2>() const;
    //! conversion to the old-style CvRect

    //! checks whether the rectangle contains the point
    bool contains(const Point_<_Tp>& pt) const;

    _Tp x, y, width, height; //< the top-left corner, as well as width and height of the rectangle
};




template<typename _Tp> inline Point_<_Tp>::Point_() :
		x(0), y(0) {
}
template<typename _Tp> inline Point_<_Tp>::Point_(_Tp _x, _Tp _y) :
		x(_x), y(_y) {
}
template<typename _Tp> inline Point_<_Tp>::Point_(const Point_& pt) :
		x(pt.x), y(pt.y) {
}

template<typename _Tp> inline Point_<_Tp>& Point_<_Tp>::operator =(
		const Point_& pt) {
	x = pt.x;
	y = pt.y;
	return *this;
}
template<typename _Tp> template<typename _Tp2> inline Point_<_Tp>::operator Point_<_Tp2>() const {
	return Point_<_Tp2>(static_cast<_Tp2>(x), static_cast<_Tp2>(y));
}

template<typename _Tp> static inline Point_<_Tp>&
operator +=(Point_<_Tp>& a, const Point_<_Tp>& b) {
	a.x = static_cast<_Tp>(a.x + b.x);
	a.y = static_cast<_Tp>(a.y + b.y);
	return a;
}

template<typename _Tp> static inline Point_<_Tp>&
operator -=(Point_<_Tp>& a, const Point_<_Tp>& b) {
	a.x = static_cast<_Tp>(a.x - b.x);
	a.y = static_cast<_Tp>(a.y - b.y);
	return a;
}

template<typename _Tp> static inline Point_<_Tp>&
operator *=(Point_<_Tp>& a, int b) {
	a.x = static_cast<_Tp>(a.x * b);
	a.y = static_cast<_Tp>(a.y * b);
	return a;
}

template<typename _Tp> static inline Point_<_Tp>&
operator *=(Point_<_Tp>& a, float b) {
	a.x = static_cast<_Tp>(a.x * b);
	a.y = static_cast<_Tp>(a.y * b);
	return a;
}

template<typename _Tp> static inline Point_<_Tp>&
operator *=(Point_<_Tp>& a, double b) {
	a.x = static_cast<_Tp>(a.x * b);
	a.y = static_cast<_Tp>(a.y * b);
	return a;
}

template<typename _Tp> static inline bool operator ==(const Point_<_Tp>& a,
		const Point_<_Tp>& b) {
	return a.x == b.x && a.y == b.y;
}

template<typename _Tp> static inline bool operator !=(const Point_<_Tp>& a,
		const Point_<_Tp>& b) {
	return a.x != b.x || a.y != b.y;
}

/////// Point_ operations

template<typename _Tp> static inline Point_<_Tp> operator +(
		const Point_<_Tp>& a, const Point_<_Tp>& b) {
	return Point_<_Tp>(static_cast<_Tp>(a.x + b.x), static_cast<_Tp>(a.y + b.y));
}

template<typename _Tp> static inline Point_<_Tp> operator -(
		const Point_<_Tp>& a, const Point_<_Tp>& b) {
	return Point_<_Tp>(static_cast<_Tp>(a.x - b.x), static_cast<_Tp>(a.y - b.y));
}

template<typename _Tp> static inline Point_<_Tp> operator -(
		const Point_<_Tp>& a) {
	return Point_<_Tp>(static_cast<_Tp>(-a.x), static_cast<_Tp>(-a.y));
}

template<typename _Tp> static inline Point_<_Tp> operator *(
		const Point_<_Tp>& a, int b) {
	return Point_<_Tp>(static_cast<_Tp>(a.x * b), static_cast<_Tp>(a.y * b));
}

template<typename _Tp> static inline Point_<_Tp> operator *(int a,
		const Point_<_Tp>& b) {
	return Point_<_Tp>(static_cast<_Tp>(b.x * a), static_cast<_Tp>(b.y * a));
}

template<typename _Tp> static inline Point_<_Tp> operator *(
		const Point_<_Tp>& a, float b) {
	return Point_<_Tp>(static_cast<_Tp>(a.x * b), static_cast<_Tp>(a.y * b));
}

template<typename _Tp> static inline Point_<_Tp> operator *(float a,
		const Point_<_Tp>& b) {
	return Point_<_Tp>(static_cast<_Tp>(b.x * a), static_cast<_Tp>(b.y * a));
}

template<typename _Tp> static inline Point_<_Tp> operator *(
		const Point_<_Tp>& a, double b) {
	return Point_<_Tp>(static_cast<_Tp>(a.x * b), static_cast<_Tp>(a.y * b));
}

template<typename _Tp> static inline Point_<_Tp> operator *(double a,
		const Point_<_Tp>& b) {
	return Point_<_Tp>(static_cast<_Tp>(b.x * a), static_cast<_Tp>(b.y * a));
}


//////////////////////////////// Size ////////////////////////////////

template<typename _Tp> inline Size_<_Tp>::Size_()
    : width(0), height(0) {}
template<typename _Tp> inline Size_<_Tp>::Size_(_Tp _width, _Tp _height)
    : width(_width), height(_height) {}
template<typename _Tp> inline Size_<_Tp>::Size_(const Size_& sz)
    : width(sz.width), height(sz.height) {}
template<typename _Tp> inline Size_<_Tp>::Size_(const Point_<_Tp>& pt) : width(pt.x), height(pt.y) {}

template<typename _Tp> template<typename _Tp2> inline Size_<_Tp>::operator Size_<_Tp2>() const
{ return Size_<_Tp2>(static_cast<_Tp2>(width), static_cast<_Tp2>(height)); }

template<typename _Tp> inline Size_<_Tp>& Size_<_Tp>::operator = (const Size_<_Tp>& sz)
{ width = sz.width; height = sz.height; return *this; }
template<typename _Tp> static inline Size_<_Tp> operator * (const Size_<_Tp>& a, _Tp b)
{ return Size_<_Tp>(a.width * b, a.height * b); }
template<typename _Tp> static inline Size_<_Tp> operator + (const Size_<_Tp>& a, const Size_<_Tp>& b)
{ return Size_<_Tp>(a.width + b.width, a.height + b.height); }
template<typename _Tp> static inline Size_<_Tp> operator - (const Size_<_Tp>& a, const Size_<_Tp>& b)
{ return Size_<_Tp>(a.width - b.width, a.height - b.height); }
template<typename _Tp> inline _Tp Size_<_Tp>::area() const { return width*height; }

template<typename _Tp> static inline Size_<_Tp>& operator += (Size_<_Tp>& a, const Size_<_Tp>& b)
{ a.width += b.width; a.height += b.height; return a; }
template<typename _Tp> static inline Size_<_Tp>& operator -= (Size_<_Tp>& a, const Size_<_Tp>& b)
{ a.width -= b.width; a.height -= b.height; return a; }

template<typename _Tp> static inline bool operator == (const Size_<_Tp>& a, const Size_<_Tp>& b)
{ return a.width == b.width && a.height == b.height; }
template<typename _Tp> static inline bool operator != (const Size_<_Tp>& a, const Size_<_Tp>& b)
{ return a.width != b.width || a.height != b.height; }

//////////////////////////////// Rect ////////////////////////////////


template<typename _Tp> inline Rect_<_Tp>::Rect_() : x(0), y(0), width(0), height(0) {}
template<typename _Tp> inline Rect_<_Tp>::Rect_(_Tp _x, _Tp _y, _Tp _width, _Tp _height) : x(_x), y(_y), width(_width), height(_height) {}
template<typename _Tp> inline Rect_<_Tp>::Rect_(const Rect_<_Tp>& r) : x(r.x), y(r.y), width(r.width), height(r.height) {}
template<typename _Tp> inline Rect_<_Tp>::Rect_(const Point_<_Tp>& org, const Size_<_Tp>& sz) :
    x(org.x), y(org.y), width(sz.width), height(sz.height) {}
template<typename _Tp> inline Rect_<_Tp>::Rect_(const Point_<_Tp>& pt1, const Point_<_Tp>& pt2)
{
    x = min(pt1.x, pt2.x); y = min(pt1.y, pt2.y);
    width = max(pt1.x, pt2.x) - x; height = max(pt1.y, pt2.y) - y;
}
template<typename _Tp> inline Rect_<_Tp>& Rect_<_Tp>::operator = ( const Rect_<_Tp>& r )
{ x = r.x; y = r.y; width = r.width; height = r.height; return *this; }

template<typename _Tp> inline Point_<_Tp> Rect_<_Tp>::tl() const { return Point_<_Tp>(x,y); }
template<typename _Tp> inline Point_<_Tp> Rect_<_Tp>::br() const { return Point_<_Tp>(x+width, y+height); }

template<typename _Tp> static inline Rect_<_Tp>& operator += ( Rect_<_Tp>& a, const Point_<_Tp>& b )
{ a.x += b.x; a.y += b.y; return a; }
template<typename _Tp> static inline Rect_<_Tp>& operator -= ( Rect_<_Tp>& a, const Point_<_Tp>& b )
{ a.x -= b.x; a.y -= b.y; return a; }

template<typename _Tp> static inline Rect_<_Tp>& operator += ( Rect_<_Tp>& a, const Size_<_Tp>& b )
{ a.width += b.width; a.height += b.height; return a; }

template<typename _Tp> static inline Rect_<_Tp>& operator -= ( Rect_<_Tp>& a, const Size_<_Tp>& b )
{ a.width -= b.width; a.height -= b.height; return a; }

template<typename _Tp> static inline Rect_<_Tp>& operator &= ( Rect_<_Tp>& a, const Rect_<_Tp>& b )
{
    _Tp x1 = max(a.x, b.x), y1 = max(a.y, b.y);
    a.width = min(a.x + a.width, b.x + b.width) - x1;
    a.height = min(a.y + a.height, b.y + b.height) - y1;
    a.x = x1; a.y = y1;
    if( a.width <= 0 || a.height <= 0 )
        a = Rect();
    return a;
}

template<typename _Tp> static inline Rect_<_Tp>& operator |= ( Rect_<_Tp>& a, const Rect_<_Tp>& b )
{
    _Tp x1 = min(a.x, b.x), y1 = min(a.y, b.y);
    a.width = max(a.x + a.width, b.x + b.width) - x1;
    a.height = max(a.y + a.height, b.y + b.height) - y1;
    a.x = x1; a.y = y1;
    return a;
}

template<typename _Tp> inline Size_<_Tp> Rect_<_Tp>::size() const { return Size_<_Tp>(width, height); }
template<typename _Tp> inline _Tp Rect_<_Tp>::area() const { return width*height; }

template<typename _Tp> template<typename _Tp2> inline Rect_<_Tp>::operator Rect_<_Tp2>() const
{ return Rect_<_Tp2>(static_cast<_Tp2>(x), static_cast<_Tp2>(y),
		static_cast<_Tp2>(width), static_cast<_Tp2>(height)); }

template<typename _Tp> inline bool Rect_<_Tp>::contains(const Point_<_Tp>& pt) const
{ return x <= pt.x && pt.x < x + width && y <= pt.y && pt.y < y + height; }

template<typename _Tp> static inline bool operator == (const Rect_<_Tp>& a, const Rect_<_Tp>& b)
{
    return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
}

template<typename _Tp> static inline bool operator != (const Rect_<_Tp>& a, const Rect_<_Tp>& b)
{
    return a.x != b.x || a.y != b.y || a.width != b.width || a.height != b.height;
}

template<typename _Tp> static inline Rect_<_Tp> operator + (const Rect_<_Tp>& a, const Point_<_Tp>& b)
{
    return Rect_<_Tp>( a.x + b.x, a.y + b.y, a.width, a.height );
}

template<typename _Tp> static inline Rect_<_Tp> operator - (const Rect_<_Tp>& a, const Point_<_Tp>& b)
{
    return Rect_<_Tp>( a.x - b.x, a.y - b.y, a.width, a.height );
}

template<typename _Tp> static inline Rect_<_Tp> operator + (const Rect_<_Tp>& a, const Size_<_Tp>& b)
{
    return Rect_<_Tp>( a.x, a.y, a.width + b.width, a.height + b.height );
}

template<typename _Tp> static inline Rect_<_Tp> operator & (const Rect_<_Tp>& a, const Rect_<_Tp>& b)
{
    Rect_<_Tp> c = a;
    return c &= b;
}

template<typename _Tp> static inline Rect_<_Tp> operator | (const Rect_<_Tp>& a, const Rect_<_Tp>& b)
{
    Rect_<_Tp> c = a;
    return c |= b;
}

template<typename _Tp> inline bool Point_<_Tp>::inside( const Rect_<_Tp>& r ) const
{
    return r.contains(*this);
}



} /* namespace daum */

#endif /* __DAUM__CORE_HPP__ */
