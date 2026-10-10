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
    assert(DispatchShortcut(bindings,masks,touch|L1|options,Mask{0}).action==2);
    const auto swapped=AssignShortcut(bindings,2,0);
    assert(DispatchShortcut(swapped,masks,touch|L1,Mask{0}).action==2);
    assert(DispatchShortcut(swapped,masks,touch|options,Mask{0}).action==0);
    std::cout << "PASS pause, return, HUD shortcuts, edge, priority, remap\n";
}
