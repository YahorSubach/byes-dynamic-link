# dynamic link class that alows to link objects and re-link them on moving
The motivation for this code was when we save a reference to a vector element and get garbage after reallocation. There are many ways to overcome such a problem. For example don't refer to vector elements :) Or use resize/reserve. However, I was curious to try to solve the original problem using Move constructors and templates support.

## examples
- base moved objects
	``` cpp
	using namespace byes::dynamic_link;
	
	struct A;
	struct B;
	struct C;
	
	struct A : public Linked<A, LinkArray<B,2>, LinkArray<C, 2>>
	{
		int a = 0;
	};
	
	struct B : public Linked<B, A>
	{
		int b = 0;
	};
	
	struct C : public Linked<C, A>
	{
		int c = 0;
	};
	
	void BaseTest()
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
	
		b1.b += 10;
		b2.b += 10;
		c1.c += 10;
		c2.c += 10;
	
		assert(b1.b == 110);
		assert(b2.b == 111);
	
		assert(c1.c == 210);
		assert(c2.c == 211);
	}
	```
- const refs
	``` cpp
	struct Dep;
	struct Common : public Linked<Common, LinkedAsConst<LinkArray<Dep, 2> >, LinkArray<Dep, 2> >
	{};
	
	struct Dep : public Linked<Dep, const Common, Common>
	{
		int d = 0;
	};
	
	
	void ConstTest()
	{
		const Common const_com{};
		Common com{};
	
		Dep dep1;
		Dep dep2;
	
		dep1.Set(const_com);
		dep2.Set(const_com);
	
		dep1.Set(com);
		dep2.Set(com);
	
		const_com.Get<Dep>(0).d = 1;
		com.Get<Dep>(0).d += 1;
	
		const_com.Get<Dep>(1).d = 1;
		com.Get<Dep>(1).d += 1;
	
		assert(dep1.d == 2);
		assert(dep2.d == 2);
	}
	```
- original vector element reference problem solution
	``` cpp
	struct VecDep;
	struct Element : public Linked<Element, LinkedAsConst<VecDep> >
	{
		int e = 0;
	};
	
	struct VecDep : public Linked<VecDep, const Element>
	{};
	
	
	void VectorTest()
	{
		std::vector<Element> vec;
		
		vec.push_back(Element());
		vec.back().e = 1;
	
		VecDep vec_dep;
		vec_dep.Set(AsConst(vec.back()));
	
		for (int i = 0; i < 256; i++)
		{
			vec.push_back(Element());
		}
	
		vec[0].e = 2;
	
		assert(vec_dep.Get<const Element>().e == 2);
	}
	```
