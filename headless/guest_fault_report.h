// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "mod_diagnostics.h"
#include <fmt/format.h>
#include "core/arm/debug.h"
#include "core/hle/kernel/k_process.h"
namespace Eden {
inline void CaptureGuestFault(Kernel::KThread* thread, unsigned core, const char* reason) {
    if (!ModDiagnostics::IsEnabled()) return;
    auto* process = thread->GetOwnerProcess();
    auto ctx = thread->GetContext();
    if (auto* interface = process->GetArmInterface(core)) interface->GetContext(ctx);
    std::string text = fmt::format("reason={} core={} thread={} name={}\nstopped_pc={:016X} sp={:016X} fp={:016X} lr={:016X} pstate={:08X}\n",
        reason, core, thread->GetThreadId(), Core::GetThreadName(thread).value_or("?"),
        ctx.pc, ctx.sp, ctx.fp, ctx.lr, ctx.pstate);
    text += fmt::format("fpcr={:08X} fpsr={:08X} tpidr={:016X}\n", ctx.fpcr, ctx.fpsr, ctx.tpidr);
    for (std::size_t i = 0; i < ctx.r.size(); ++i) text += fmt::format("x{}={:016X}\n", i, ctx.r[i]);
    const auto modules = Core::FindModules(process);
    if (modules.empty()) text += fmt::format("module base={:016X} name=[process-entry]\n", GetInteger(process->GetEntryPoint()));
    for (const auto& [base, name] : modules)
        text += fmt::format("module base={:016X} name={}\n", base, name);
    for (const auto& frame : Core::GetBacktraceFromContext(process, ctx))
        text += fmt::format("frame pc={:016X} {}+{:X} {}\n", frame.original_address, frame.module, frame.offset, frame.name);
    ModDiagnostics::Write("guest-fault", text, true);
}
}
