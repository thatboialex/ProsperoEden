// SPDX-License-Identifier: GPL-3.0-or-later
#include "shortcut_dispatch.h"
#include <cassert>
#include <cstdint>
#include <iostream>
int main() {
    using namespace Eden;
    using Mask = std::uint32_t;
    constexpr Mask touch=1u, create=2u, L1=4u, R1=8u, options=16u;
    constexpr std::array<Mask,6> masks{touch|L1,touch|R1,touch|options,create|L1,create|R1,create|options};
    constexpr auto bindings=kDefaultShortcuts;
    static_assert(DispatchShortcut(bindings,masks,touch|options,Mask{0}).action == 2);
    assert(DispatchShortcut(bindings,masks,touch|options,Mask{0}).activated);
    assert(!DispatchShortcut(bindings,masks,touch|options,touch|options).activated);
    assert(DispatchShortcut(bindings,masks,touch|options,Mask{0}).consumed==(touch|options));
    assert(DispatchShortcut(bindings,masks,touch|L1,Mask{0}).action==0);
    assert(DispatchShortcut(bindings,masks,touch|R1,Mask{0}).action==1);
    assert(DispatchShortcut(bindings,masks,touch,Mask{0}).action==-1);
    assert(DispatchShortcut(bindings,masks,create|options,Mask{0}).action==3);
    assert(!DispatchShortcut(bindings,masks,create|options,create|options).activated);
    assert(DispatchShortcut(bindings,masks,touch|L1|options,Mask{0}).action==2);
    const auto swapped=AssignShortcut(bindings,2,0);
    assert(DispatchShortcut(swapped,masks,touch|L1,Mask{0}).action==2);
    assert(DispatchShortcut(swapped,masks,touch|options,Mask{0}).action==0);
    constexpr Mask L2=32u,R2=64u,L3=128u,R3=256u;
    constexpr auto padmask = [=](ShortcutButton b) -> Mask {
        switch (b) {
        case ShortcutButton::touchpad: return touch;
        case ShortcutButton::create: return create;
        case ShortcutButton::options: return options;
        case ShortcutButton::l1: return L1;
        case ShortcutButton::r1: return R1;
        case ShortcutButton::l2: return L2;
        case ShortcutButton::r2: return R2;
        case ShortcutButton::l3: return L3;
        case ShortcutButton::r3: return R3;
        }
        return 0;
    };
    std::array<Mask, kShortcutChords.size()> extended{};
    for (std::size_t i = 0; i < extended.size(); ++i)
        extended[i] = padmask(kShortcutChords[i].first) | padmask(kShortcutChords[i].second);
    auto custom = AssignShortcut(bindings, 2, 16);
    assert(DispatchShortcut(custom, extended, L3|R3, Mask{0}).action == 2);
    assert(DispatchShortcut(custom, extended, L3|R3, Mask{0}).activated);
    assert(!DispatchShortcut(custom, extended, L3|R3, L3|R3).activated);
    custom = AssignShortcut(custom, 3, 15);
    assert(DispatchShortcut(custom, extended, L2|R2, Mask{0}).action == 3);
    custom = AssignShortcut(custom, 0, 17);
    assert(DispatchShortcut(custom, extended, L1|L3, Mask{0}).action == 0);
    std::cout << "PASS L3 R3 L1 R1 L2 R2 and four hotkey actions\n";
}
