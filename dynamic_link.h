#ifndef BYES_DYNAMIC_LINK_H
#define BYES_DYNAMIC_LINK_H

#include <new>
#include <stdexcept>

namespace byes {

	template<class LinkFromType, class LinkToType, typename ID = void>
	class DynamicLink
	{

	private:
		LinkToType* link_;

	protected:
		DynamicLink() : link_(nullptr) {}

	public:

		bool Has() const { return link_ != nullptr; }
		LinkToType& Get() { return *link_; }
		const LinkToType& Get() const { return *link_; }

		void Set(LinkToType& to_link)
		{
			if (link_ != &to_link)
			{
				link_ = &to_link;
				to_link.DynamicLink<LinkToType, LinkFromType, ID>::Set(*reinterpret_cast<LinkFromType*>(this));
			}
		}

		DynamicLink(const LinkToType& to_link) : link_(&to_link) {}

		DynamicLink(const DynamicLink<LinkFromType, LinkToType, ID>&)
		{
			link_ = nullptr;
		}

		DynamicLink(DynamicLink<LinkFromType, LinkToType, ID>&& moved_link)
		{
			if (moved_link.Has())
			{
				link_ = &(moved_link.Get());
				moved_link.Get().DynamicLink<LinkToType, LinkFromType, ID>::Set(*reinterpret_cast<LinkFromType>(this));
			}
			else
			{
				link_ = nullptr;
			}
		}

		DynamicLink& operator=(const DynamicLink<LinkFromType, LinkToType, ID>&)
		{
			link_ = nullptr;

			return *this;
		}

		DynamicLink& operator=(DynamicLink<LinkFromType, LinkToType, ID>&& moved_link)
		{
			if (moved_link.Has())
			{
				link_ = &(moved_link.Get());
				moved_link.Get().DynamicLink<LinkToType, LinkFromType, ID>::Set(*reinterpret_cast<LinkFromType*>(this));
			}
			else
			{
				link_ = nullptr;
			}

			return *this;
		}
	};
}

#endif // BYES_DYNAMIC_LINK_H