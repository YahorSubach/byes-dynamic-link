#ifndef BYES_DYNAMIC_LINK_H
#define BYES_DYNAMIC_LINK_H

#include <new>
#include <stdexcept>

namespace byes {

	template<typename T>
	struct remove_const
	{
		typedef T Type;
	};

	template<typename T>
	struct remove_const<const T>
	{
		typedef T Type;
	};


	template<class ToType, size_t link_cnt = 1>
	struct LinkArray
	{

	};


	template<class FromType, class... ToTypes>
	class LinkedInternal {};

	template<class FromType, class ToTypeFirst, class... ToTypes>
	class LinkedInternal<FromType, ToTypeFirst, ToTypes...> : public Linked<FromType, LinkArray<ToTypeFirst>, ToTypes...>
	{};

	template<class FromType, class ToTypeFirst, size_t first_array_cnt, class... ToTypes>
	class LinkedInternal<FromType, LinkArray<ToTypeFirst, first_array_cnt>, ToTypes...> : public Linked<FromType, ToTypes...>
	{

	private:

		std::pair<ToTypeFirst*, size_t> links_and_remote_indices_[first_array_cnt];

	protected:

		Linked<FromType, LinkArray<ToTypeFirst, first_array_cnt>, ToTypes...>() : links_() {}

		ToTypeFirst& Get(size_t index)
		{
			if (index < first_array_cnt)
			{
				return *links_and_remote_indices_[index].first;
			}
			throw std::out_of_range();
		}

		size_t Append(ToTypeFirst& to_link)
		{
			for (size_t i = 0; i < first_array_cnt; i++)
			{
				if (links_and_remote_indices_[index].first == &to_link)
					return index;
			}

			size_t index = 0;
			for (; index < first_array_cnt && links_and_remote_indices_[index] != nullptr; index++);

			if (index == first_array_cnt) throw std::length_error();

			ToTypeFirst& to_link_mutable = const_cast<ToTypeFirst&>(to_link);
			links_and_remote_indices_[index].first = &to_link;
			links_and_remote_indices_[index].second = to_link_mutable.Append<FromType>(static_cast<FromType&>(*this));
		}

		void Reset(size_t index)
		{
			if (index < first_array_cnt)
			{
				if (links_and_remote_indices_->first != nullptr)
				{
					ToTypeFirst& to_link_mutable = const_cast<ToTypeFirst&>(*(links_and_remote_indices_[index]->first));
					links_and_remote_indices_->first = nullptr;
					to_link_mutable.Reset<FromType>(links_and_remote_indices_[index]->second);
				}
				return;
			}
			throw std::out_of_range();
		}

		void Set(ToTypeFirst& to_link, size_t index)
		{
			if (index < first_array_cnt)
			{
				if (links_[index].first != nullptr && links_[index].first != to_link)
				{
					Reset(index);
				}

				if (links_and_remote_indices_[index].first == nulptr)
				{
					links_and_remote_indices_[index].first = &to_link;

					ToTypeFirst& to_link_mutable = const_cast<ToTypeFirst&>(to_link);
					links_and_remote_indices_[index].second = to_link_mutable.Append<FromType>(static_cast<FromType&>(*this));
				}

				return;
			}

			throw std::out_of_range();
		}

		Linked<FromType, LinkArray<ToTypeFirst, first_array_cnt>, ToTypes...>& operator=(const Linked<FromType, LinkArray<ToTypeFirst, first_array_cnt>, ToTypes...>& copied)
		{
			links_and_remote_indices_ = copied.links_and_remote_indices;

			for (size_t i = 0; i < first_array_cnt; i++)
			{
				if (links_and_remote_indices_[i] != nullptr)
				{
					ToTypeFirst& to_link_mutable = const_cast<ToTypeFirst&>(*(links_and_remote_indices_[i]));
					links_and_remote_indices_[i].second = to_link_mutable.Append<FromType>(static_cast<FromType&>(*this))
				}
			}

			return *this;
		}

		Linked<FromType, LinkArray<ToTypeFirst, first_array_cnt>, ToTypes...>& operator=(Linked<FromType, LinkArray<ToTypeFirst, first_array_cnt>, ToTypes...>&& moved)
		{
			links_and_remote_indices_ = moved.links_and_remote_indices;

			for (size_t i = 0; i < first_array_cnt; i++)
			{
				if (links_and_remote_indices_[i] != nullptr)
				{
					ToTypeFirst& to_link_mutable = const_cast<ToTypeFirst&>(*(links_and_remote_indices_[i]));
					to_link_mutable.Set<FromType>(static_cast<FromType&>(*this), links_and_remote_indices_[i].second);
				}
			}

			return *this;
		}

	};

	template<class FromType, class... ToTypes>
	class Linked: private LinkedInternal<FromType, ToTypes>
	{
	public:

		template<class ToType>
		auto&& Get() { }

	};


	//template<class LinkFromType, class LinkToType, typename ID = void>
	//class DynamicLink
	//{
	//	
	//}

	//private:
	//	LinkToType* link_;

	//protected:
	//	DynamicLink() : link_(nullptr) {}

	//public:

	//	bool Has() const { return link_ != nullptr; }
	//	LinkToType& Get() { return *link_; }
	//	const LinkToType& Get() const { return *link_; }

	//	void Set(LinkToType& to_link)
	//	{
	//		if (link_ != &to_link)
	//		{
	//			link_ = &to_link;
	//			to_link.DynamicLink<LinkToType, LinkFromType, ID>::Set(static_cast<LinkFromType&>(*this));
	//		}
	//	}

	//	DynamicLink(const LinkToType& to_link) : link_(&to_link) {}

	//	DynamicLink(const DynamicLink<LinkFromType, LinkToType, ID>&)
	//	{
	//		link_ = nullptr;
	//	}

	//	DynamicLink(DynamicLink<LinkFromType, LinkToType, ID>&& moved_link)
	//	{
	//		if (moved_link.Has())
	//		{
	//			link_ = &(moved_link.Get());
	//			moved_link.Get().DynamicLink<LinkToType, LinkFromType, ID>::Set(static_cast<LinkFromType&>(*this));
	//		}
	//		else
	//		{
	//			link_ = nullptr;
	//		}
	//	}

	//	DynamicLink& operator=(const DynamicLink<LinkFromType, LinkToType, ID>&)
	//	{
	//		link_ = nullptr;

	//		return *this;
	//	}

	//	DynamicLink& operator=(DynamicLink<LinkFromType, LinkToType, ID>&& moved_link)
	//	{
	//		if (moved_link.Has())
	//		{
	//			link_ = &(moved_link.Get());
	//			moved_link.Get().DynamicLink<LinkToType, LinkFromType, ID>::Set(static_cast<LinkFromType&>(*this));
	//		}
	//		else
	//		{
	//			link_ = nullptr;
	//		}

	//		return *this;
	//	}
	//};
}

#endif // BYES_DYNAMIC_LINK_H