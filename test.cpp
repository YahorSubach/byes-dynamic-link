// dynamic_link.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <cassert>
#include <iostream>

#include "dynamic_link.h"

struct A;
struct B;
struct C;

struct ExplicitId;

struct A : public byes::DynamicLink<A, B>, byes::DynamicLink<A, C, ExplicitId>
{
	int a = 0;
};

struct B : public byes::DynamicLink<B, A>
{
	int b = 0;
};

struct C : public byes::DynamicLink<C, A, ExplicitId>
{
	int c = 0;
};


int main()
{
	A a;
	B b;
	C c;

	{
		A a_local;
		B b_local;
		C c_local;

		a_local.byes::DynamicLink<A, B>::Set(b_local);
		a_local.byes::DynamicLink<A, C, ExplicitId>::Set(c_local);

		a = std::move(a_local);
		b = std::move(b_local);
		c = std::move(c_local);
	}

	a.byes::DynamicLink<A, B>::Get().b = 1;
	a.byes::DynamicLink<A, C, ExplicitId>::Get().c = 1;

	c.Get().a = 1;

	assert(a.a == 1);
	assert(b.b == 1);
	assert(c.c == 1);

}
