#pragma once
/******************************************************************************/
/*!
\file	Vector3.h
\author Wen Sheng Tang \ Lim Chian Song
\par	email: tang_wen_sheng\@nyp.edu.sg
\brief
Struct to define a 3D vector
*/
/******************************************************************************/
#include <iostream>
#include "MyMath.h"

/******************************************************************************/
/*!
		Class Vector3:
\brief	Defines a 3D vector and its methods
*/
/******************************************************************************/
struct Vector3
{
	float x, y, z;

	explicit Vector3(float a = 0.0, float b = 0.0, float c = 0.0);
	Vector3(const Vector3 &rhs);
	~Vector3();
	
	void Set( float a = 0, float b = 0, float c = 0 ); //Set all data
	void SetZero( void ); //Set all data to zero
	bool IsZero( void ) const; //Check if data is zero

	Vector3 operator+( const Vector3& rhs ) const; //Vector addition
	Vector3& operator+=( const Vector3& rhs ); 
	
	Vector3 operator-( const Vector3& rhs ) const; //Vector subtraction
	Vector3& operator-=( const Vector3& rhs );
	
	Vector3 operator-( void ) const; //Unary negation
	
	Vector3 operator*( float scalar ) const; //Scalar multiplication
	Vector3& operator*=( float scalar );

	bool operator==( const Vector3& rhs ) const; //Equality check
	bool operator!= ( const Vector3& rhs ) const; //Inequality check

	Vector3& operator=(const Vector3& rhs); //Assignment operator

	float Length( void ) const; //Get magnitude
	float LengthSquared (void ) const; //Get square of magnitude
	float Distance(const Vector3& rhs) const; //Get the distance 
	float DistanceSquared(const Vector3& rhs) const; //Get the distance squared 
	static float Distance(const Vector3& lhs, const Vector3& rhs);
	static float DistanceSquared(const Vector3& lhs, const Vector3& rhs);

	float Dot( const Vector3& rhs ) const; //Dot product
	Vector3 Cross( const Vector3& rhs ) const; //Cross product
	
	//Return a copy of this vector, normalized
	Vector3 Normalized( void ) const;
	
	//Normalize this vector and return a reference to it
	Vector3& Normalize( void );

	Vector3 operator/(float scalar) const
	{
		return Vector3(x / scalar, y / scalar, z / scalar);
	}

private:
	//use this to check if 2 floats are equal
	bool IsEqual(float a, float b) const;
};

Vector3 operator*(float scalar, const Vector3& rhs);
std::ostream& operator<< (std::ostream& os, const Vector3& rhs);
