// dynamic_link.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <cassert>
#include <iostream>

#include "dynamic_link.h"

struct A;
struct B;
struct C;

struct ExplicitId;

struct A : public byes::Linked<A, byes::LinkArray<B,2>, byes::LinkArray<C, 2>>
{
	int a = 0;
};

struct B : public byes::Linked<B, A>
{
	int b = 0;
};

struct C : public byes::Linked<C, A>
{
	int c = 0;
};

struct Dep;
struct Common : public byes::Linked<Common, byes::LinkArray<Dep, 16>>
{
	int c;
};

struct Dep : public byes::Linked<Dep, Common>
{
	int d;
};


int main()
{
	A a;

	B b1;
	B b2;

	C c1;
	C c2;

	{
		B b1_local;
		B b2_local;

		C c1_local;
		C c2_local;

		a.Append<B>(b1_local);
		a.Append<B>(b2_local);

		a.Append<C>(c1_local);
		a.Append<C>(c2_local);

		b1_local.b = 100;
		b2_local.b = 101;

		c1_local.c = 200;
		c2_local.c = 201;

		b1 = std::move(b1_local);
		b2 = std::move(b2_local);

		c1 = std::move(c1_local);
		c2 = std::move(c2_local);
	}

	b1.b+=10;
	b2.b += 10;
	c1.c += 10;
	c2.c += 10;

	
	const Common com();

	Dep dep1;
	Dep dep2;

	dep1.Set(com);
	dep2.Set(com);


	//byes::LinksHolder<A, B, 1> lh;
	//byes::LinksHolder<A, B, 1> lh2 = lh;

	//C c;

	//{
	//	A a_local;
	//	B b_local;
	//	C c_local;

	//	a_local.byes::DynamicLink<A, B>::Set(b_local);
	//	a_local.byes::DynamicLink<A, C, ExplicitId>::Set(c_local);

	//	a = std::move(a_local);
	//	b = std::move(b_local);
	//	c = std::move(c_local);
	//}

	//a.byes::DynamicLink<A, B>::Get().b = 1;
	//a.byes::DynamicLink<A, C, ExplicitId>::Get().c = 1;

	//c.Get().a = 1;

	//assert(a.a == 1);
	//assert(b.b == 1);
	//assert(c.c == 1);

}
