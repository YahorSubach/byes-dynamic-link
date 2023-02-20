#ifndef BYES_DYNAMIC_LINK_H
#define BYES_DYNAMIC_LINK_H

#include <new>
#include <stdexcept>

namespace byes {

	namespace dynamic_link
	{
		template<typename RefType>
		struct RemoveConst
		{
			using Type = RefType;
		};

		template<typename RefType>
		struct RemoveConst<const RefType>
		{
			using Type = RefType;
		};

		template<class ToType, size_t link_cnt = 1>
		struct LinkArray {};

		template<class ToType>
		struct LinkedAsConst {};

		template<class ToType>
		const ToType& AsConst(ToType& obj) { return obj; }

		template<class ConstnessFromType, class ConstnessToType, size_t link_cnt = 0>
		class LinksHolder
		{

		private:

			std::pair<ConstnessToType*, size_t> links_and_remote_indices_[link_cnt];

			void NullPtrFill() noexcept
			{
				for (size_t index = 0; index < link_cnt; index++)
				{
					links_and_remote_indices_[index].first = nullptr;
				}
			}

			inline void CopyLinksAndRemoteIndices(const LinksHolder& source) noexcept
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
						links_and_remote_indices_[index].second = links_and_remote_indices_[index].first->Append<ConstnessFromType>(static_cast<ConstnessFromType&>(*this));
					}
				}
			}

			inline void SetAllRemoteToThis()
			{
				for (size_t index = 0; index < link_cnt; index++)
				{
					if (links_and_remote_indices_[index].first != nullptr)
					{
						links_and_remote_indices_[index].first->Set<ConstnessFromType>(static_cast<ConstnessFromType&>(*this), links_and_remote_indices_[index].second);
					}
				}
			}

		public:

			LinksHolder() noexcept
			{
				NullPtrFill();
			}

			LinksHolder(const LinksHolder& copied)
			{
				CopyLinksAndRemoteIndices(copied);
				AppendThisToAllRemote();
			}

			LinksHolder(LinksHolder&& moved) noexcept
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

			LinksHolder& operator=(LinksHolder&& moved) noexcept
			{
				CopyLinksAndRemoteIndices(moved);
				SetAllRemoteToThis();
				moved.NullPtrFill();

				return *this;
			}

			ConstnessToType& Get(size_t index) const
			{
				if (index < link_cnt)
				{
					return *links_and_remote_indices_[index].first;
				}
				throw std::out_of_range("index");
			}

			size_t Append(ConstnessToType& to_link)
			{
				for (size_t index = 0; index < link_cnt; index++)
				{
					if (links_and_remote_indices_[index].first == &to_link)
						return index;
				}

				size_t index = 0;
				for (; index < link_cnt && links_and_remote_indices_[index].first != nullptr; index++);

				if (index == link_cnt) throw std::length_error("attempt to append new link when all links have been already assigned");

				links_and_remote_indices_[index].first = &to_link;

				LinksHolder<ConstnessToType, ConstnessFromType, ConstnessToType::template LinkCount<ConstnessToType, ConstnessFromType>::value>& mutable_link_holder =
					static_cast<LinksHolder<ConstnessToType, ConstnessFromType, ConstnessToType::template LinkCount<ConstnessToType, ConstnessFromType>::value>&>(
						const_cast<RemoveConst<ConstnessToType>::Type&>(*links_and_remote_indices_[index].first)
						);

				links_and_remote_indices_[index].second = mutable_link_holder.Append(static_cast<ConstnessFromType&>(*this));

				return index;
			}

			void Reset(size_t index)
			{
				if (index < link_cnt)
				{
					if (links_and_remote_indices_[index].first != nullptr)
					{

						LinksHolder<ConstnessToType, ConstnessFromType, ConstnessToType::template LinkCount<ConstnessToType, ConstnessFromType>::value>& mutable_link_holder =
							static_cast<LinksHolder<ConstnessToType, ConstnessFromType, ConstnessToType::template LinkCount<ConstnessToType, ConstnessFromType>::value>&>
							(
								const_cast<RemoveConst<ConstnessToType>::Type&>(*links_and_remote_indices_[index].first)
							);

						links_and_remote_indices_[index].first = nullptr;
						mutable_link_holder.Reset(links_and_remote_indices_[index].second);
					}
					return;
				}
				throw std::out_of_range("index");
			}

			void Set(ConstnessToType& to_link, size_t index)
			{
				if (index < link_cnt)
				{
					Reset(index);

					links_and_remote_indices_[index].first = &to_link;

					LinksHolder<ConstnessToType, ConstnessFromType, ConstnessToType::template LinkCount<ConstnessToType, ConstnessFromType>::value>& mutable_link_holder =
						static_cast<LinksHolder<ConstnessToType, ConstnessFromType, ConstnessToType::template LinkCount<ConstnessToType, ConstnessFromType>::value>&>
						(
							const_cast<RemoveConst<ConstnessToType>::Type&>(*links_and_remote_indices_[index].first)
							);

					links_and_remote_indices_[index].second = mutable_link_holder.Append(static_cast<ConstnessFromType&>(*this));

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

		public:
			template<class ConstnessFromType, class ConstnessToType >
			struct LinkCount
			{
				static const size_t value = 0;
			};

			template<>
			struct LinkCount<ConstnessFromType, ConstnessToType>
			{
				static const size_t value = link_cnt;
			};
		};

		template<class FromType, class ToType>
		class LinksHolder<FromType, LinkedAsConst<ToType>, 0> : public LinksHolder<const FromType, ToType, 0>
		{};

		template<class FromType, class ToType>
		class LinksHolder<FromType, ToType, 0> : public LinksHolder<FromType, ToType, 1>
		{};

		template<class FromType, class ToType, size_t link_cnt>
		class LinksHolder<FromType, LinkArray<ToType, link_cnt>, 0> : public LinksHolder<FromType, ToType, link_cnt>
		{};

		template<class FromType, class... ToTypes>
		class LinkedInternal
		{
		protected:

			template<class ConstnessFromType, class ConstnessToType >
			struct LinkCount
			{
				static const size_t value = 0;
			};
		};

		template<class FromType, class ToTypeFirst, class... ToTypes>
		class LinkedInternal<FromType, ToTypeFirst, ToTypes...>: public LinksHolder<FromType, ToTypeFirst>, public LinkedInternal<FromType, ToTypes...>
		{
		protected:

			template<class ConstnessFromType, class ConstnessToType >
			struct LinkCount
			{
				static const size_t value = 
					LinksHolder<FromType, ToTypeFirst>::template LinkCount<ConstnessFromType, ConstnessToType>::value +
					LinkedInternal<FromType, ToTypes...>::template LinkCount<ConstnessFromType, ConstnessToType>::value;
			};
		};

		template<class FromType, class... ToTypes>
		class Linked : public LinkedInternal<FromType, ToTypes...>
		{
		public:

			Linked() = default;

			template<typename FirstType, typename ... OtherTypes>
			Linked(FirstType& first_ref, OtherTypes ... other_refs) : Linked(other_refs...)
			{
				Set<FirstType>(first_ref);
			}

			template<class ConstnessFromType, class ConstnessToType >
			struct LinkCount
			{
				static const size_t value = LinkedInternal<FromType, ToTypes...>::template LinkCount<ConstnessFromType, ConstnessToType>::value;
			};

			template<class ToType>
			ToType& Get(size_t index = 0) { return LinksHolder<FromType, ToType, LinkedInternal<FromType, ToTypes...>::template LinkCount<FromType, ToType>::value>::Get(index); }

			template<class ToType>
			ToType& Get(size_t index = 0) const { return LinksHolder<const FromType, ToType, LinkedInternal<FromType, ToTypes...>::template LinkCount<const FromType, ToType>::value>::Get(index); }

			template<class ToType>
			void Set(ToType& to_link, size_t index = 0) { LinksHolder<FromType, ToType, LinkedInternal<FromType, ToTypes...>::template LinkCount<FromType, ToType>::value>::Set(to_link, index); }

			template<class ToType>
			void Set(ToType& to_link, size_t index = 0) const { LinksHolder<const FromType, ToType, LinkedInternal<FromType, ToTypes...>::template LinkCount<const FromType, ToType>::value>::Set(to_link, index); }

			template<class ToType>
			size_t Append(ToType& to_link) { return LinksHolder<FromType, ToType, LinkedInternal<FromType, ToTypes...>::template LinkCount<FromType, ToType>::value>::Append(to_link); }

			template<class ToType>
			size_t Append(ToType& to_link) const { return LinksHolder<const FromType, ToType, LinkedInternal<FromType, ToTypes...>::template LinkCount<const FromType, ToType>::value>::Append(to_link); }

			template<class ToType>
			void Reset(size_t index = 0) { LinksHolder<FromType, ToType, LinkedInternal<FromType, ToTypes...>::template LinkCount<FromType, ToType>::value>::Reset(index); }

			template<class ToType>
			void Reset(size_t index = 0) const { LinksHolder<const FromType, ToType, LinkedInternal<FromType, ToTypes...>::template LinkCount<const FromType, ToType>::value>::Reset(index); }
		};
	}
}

#endif // BYES_DYNAMIC_LINK_H