#ifndef BYES_DYNAMIC_LINK_H
#define BYES_DYNAMIC_LINK_H

#include <new>
#include <stdexcept>

namespace byes {

	template<typename RefType>
	struct RemoveConst
	{
		typedef RefType Type;
	};

	template<typename RefType>
	struct RemoveConst<const RefType>
	{
		typedef RefType Type;
	};

	template<class ToType, size_t link_cnt = 1>
	struct LinkArray
	{

	};

	template<class FromType, class ToType, size_t link_cnt>
	class LinksHolder
	{

	private:

		std::pair<ToType*, size_t> links_and_remote_indices_[link_cnt];

		void NullPtrFill()
		{
			for (size_t index = 0; index < link_cnt; index++)
			{
				links_and_remote_indices_[index].first = nullptr;
			}
		}

		inline void CopyLinksAndRemoteIndices(const LinksHolder& source)
		{
			for (size_t index = 0; index < link_cnt; index++)
			{
				links_and_remote_indices_[index] = source.links_and_remote_indices_[index];
			}
		}

		inline void AppendThisToAllRemote()
		{
			for (size_t index = 0; index < link_cnt; index++)
			{
				if (links_and_remote_indices_[index].first != nullptr)
				{
					links_and_remote_indices_[index].second = links_and_remote_indices_[index].first->Append<FromType>(static_cast<FromType&>(*this));
				}
			}
		}

		inline void SetAllRemoteToThis()
		{
			for (size_t index = 0; index < link_cnt; index++)
			{
				if (links_and_remote_indices_[index].first != nullptr)
				{
					links_and_remote_indices_[index].first->Set<FromType>(static_cast<FromType&>(*this), links_and_remote_indices_[index].second);
				}
			}
		}

	public:

		LinksHolder()
		{
			NullPtrFill();
		}

		LinksHolder(const LinksHolder& copied)
		{
			CopyLinksAndRemoteIndices(copied);
			AppendThisToAllRemote();
		}

		LinksHolder(LinksHolder&& moved)
		{
			CopyLinksAndRemoteIndices(moved);
			SetAllRemoteToThis();
			moved.NullPtrFill();
		}

		LinksHolder& operator=(const LinksHolder& copied)
		{
			CopyLinksAndRemoteIndices(copied);
			AppendThisToAllRemote();

			return *this;
		}

		LinksHolder& operator=(LinksHolder&& moved)
		{
			CopyLinksAndRemoteIndices(moved);
			SetAllRemoteToThis();
			moved.NullPtrFill();

			return *this;
		}

		ToType& Get(size_t index) const
		{
			if (index < link_cnt)
			{
				return *links_and_remote_indices_[index].first;
			}
			throw std::out_of_range("index");
		}

		size_t Append(ToType& to_link)
		{
			for (size_t index = 0; index < link_cnt; index++)
			{
				if (links_and_remote_indices_[index].first == &to_link)
					return index;
			}

			size_t index = 0;
			for (; index < link_cnt && links_and_remote_indices_[index].first != nullptr; index++);

			if (index == link_cnt) throw std::length_error("attempt to append new link when all links have been already assigned");

			typename RemoveConst<ToType>::Type& to_link_mutable = const_cast<RemoveConst<ToType>::Type&>(to_link);
			links_and_remote_indices_[index].first = &to_link;
			links_and_remote_indices_[index].second = to_link_mutable.Append<FromType>(static_cast<FromType&>(*this));

			return index;
		}

		void Reset(size_t index)
		{
			if (index < link_cnt)
			{
				if (links_and_remote_indices_[index].first != nullptr)
				{
					typename RemoveConst<ToType>::Type& to_link_mutable = const_cast<RemoveConst<ToType>::Type&>(*links_and_remote_indices_[index].first);

					links_and_remote_indices_[index].first = nullptr;
					to_link_mutable.Reset<FromType>(links_and_remote_indices_[index].second);
				}
				return;
			}
			throw std::out_of_range("index");
		}

		void Set(ToType& to_link, size_t index)
		{
			if (index < link_cnt)
			{
				Reset(index);

				links_and_remote_indices_[index].first = &to_link;

				typename RemoveConst<ToType>::Type& to_link_mutable = const_cast<RemoveConst<ToType>::Type&>(to_link);

				links_and_remote_indices_[index].second = to_link_mutable.Append<FromType>(static_cast<FromType&>(*this));

				return;
			}

			throw std::out_of_range("index");
		}

		~LinksHolder()
		{
			for (size_t index = 0; index < link_cnt; index++)
			{
				Reset(index);
			}
		}
	};


	template<class FromType, class... ToTypes>
	class LinkedInternal 
	{
	protected:
		template<typename ToType>
		struct LinkCount
		{
			static const size_t value = 0;
		};
	};

	template<class FromType, class ToTypeFirst, class... ToTypes>
	class LinkedInternal<FromType, ToTypeFirst, ToTypes...> : public LinkedInternal<FromType, LinkArray<ToTypeFirst, 1>, ToTypes...>
	{
	protected:

		template<typename ToType>
		struct LinkCount
		{
			static const size_t value = LinkedInternal<FromType, LinkArray<ToTypeFirst>, ToTypes...>::template LinkCount<ToType>::value;
		};
	};

	template<class FromType, class ToTypeFirst, size_t first_array_cnt, class... ToTypes>
	class LinkedInternal<FromType, LinkArray<ToTypeFirst, first_array_cnt>, ToTypes...> : public LinksHolder<FromType, ToTypeFirst, first_array_cnt>, public LinkedInternal<FromType, ToTypes...>
	{
	protected:

		template<typename ToType>
		struct LinkCount
		{
			static const size_t value = LinkedInternal<FromType, ToTypes...>::template LinkCount<ToType>::value;
		};

		template<>
		struct LinkCount<ToTypeFirst>
		{
			static const size_t value = first_array_cnt;
		};
	};

	template<class FromType, class... ToTypes>
	class Linked: public LinkedInternal<FromType, ToTypes...>
	{
	public:

		Linked() = default;

		template<typename FirstType, typename ... OtherTypes>
		Linked(FirstType& first_ref, OtherTypes ... other_refs): Linked(other_refs...)
		{
			Set<FirstType>(first_ref);
		}

		template<typename ToType>
		struct LinkCount
		{
			static const size_t value = LinkedInternal<FromType, ToTypes...>::template LinkCount<ToType>::value;
		};

		template<class ToType>
		ToType& Get(size_t index = 0) const { return LinksHolder<FromType, ToType, LinkedInternal<FromType, ToTypes...>::template LinkCount<ToType>::value>::Get(index); }

		template<class ToType>
		void Set(ToType& to_link, size_t index = 0) { LinksHolder<FromType, ToType, LinkedInternal<FromType, ToTypes...>::template LinkCount<ToType>::value>::Set(to_link, index); }

		template<class ToType>
		size_t Append(ToType& to_link) { return LinksHolder<FromType, ToType, LinkedInternal<FromType, ToTypes...>::template LinkCount<ToType>::value>::Append(to_link); }

		template<class ToType>
		void Reset(size_t index = 0) { LinksHolder<FromType, ToType, LinkedInternal<FromType, ToTypes...>::template LinkCount<ToType>::value>::Reset(index); }
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