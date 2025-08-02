#pragma once
#include <expected>
#include <utility>

inline constexpr struct bind_to_t {
    template <typename ChannelLike, typename RegisterLike>
    auto operator()(ChannelLike&& ch, RegisterLike&& reg) const
        noexcept(noexcept(tag_invoke(*this, std::forward<ChannelLike>(ch), std::forward<RegisterLike>(reg))))
            -> decltype(tag_invoke(*this, std::forward<ChannelLike>(ch), std::forward<RegisterLike>(reg)))
    {
        return tag_invoke(*this, std::forward<ChannelLike>(ch), std::forward<RegisterLike>(reg));
    }
} bind_to;
