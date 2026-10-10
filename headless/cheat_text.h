// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

// Switch cheat collections often contain comments or a UTF-8 BOM. Eden's
// TextCheatParser treats everything except brackets, hexadecimal words, and
// whitespace as an error and discards the whole file. The library's list
// parser is more permissive, so entries could be shown yet never reach the VM.
// Strip presentation-only text, never change or guess an opcode word.
#include <cctype>
#include <string>
#include <string_view>

namespace Eden::Cheats {
inline std::string_view TrimCheatLine(std::string_view line) {
    while (!line.empty() && std::isspace(static_cast<unsigned char>(line.front())))
        line.remove_prefix(1);
    while (!line.empty() && std::isspace(static_cast<unsigned char>(line.back())))
        line.remove_suffix(1);
    return line;
}

inline std::string NormalizeText(std::string_view input) {
    if (input.size() >= 3 &&
        static_cast<unsigned char>(input[0]) == 0xEF &&
        static_cast<unsigned char>(input[1]) == 0xBB &&
        static_cast<unsigned char>(input[2]) == 0xBF)
        input.remove_prefix(3);

    std::string output;
    output.reserve(input.size());
    std::size_t at = 0;
    while (at < input.size()) {
        const auto end = input.find('\n', at);
        const auto count = end == std::string_view::npos ? input.size() - at : end - at;
        auto line = TrimCheatLine(input.substr(at, count));
        at += count + (end == std::string_view::npos ? 0 : 1);
        if (line.empty() || line.starts_with("//") || line.starts_with("#") ||
            line.starts_with(";"))
            continue;

        // The name may itself contain '#', '//', or ';'. Preserve everything
        // inside [cheat] and {master} labels, but drop trailing annotations.
        if (line.front() == '[' || line.front() == '{') {
            const auto closing = line.find(line.front() == '[' ? ']' : '}');
            if (closing != std::string_view::npos)
                line = line.substr(0, closing + 1);
        } else {
            // An opcode line is still passed unchanged if it has malformed
            // hex; the real parser must reject it rather than execute a
            // guessed/corrected instruction.
            auto comment = line.find("//");
            const auto hash = line.find('#');
            const auto semicolon = line.find(';');
            if (hash != std::string_view::npos &&
                (comment == std::string_view::npos || hash < comment))
                comment = hash;
            if (semicolon != std::string_view::npos &&
                (comment == std::string_view::npos || semicolon < comment))
                comment = semicolon;
            if (comment != std::string_view::npos)
                line = TrimCheatLine(line.substr(0, comment));
        }
        if (!line.empty()) {
            output.append(line);
            output.push_back('\n');
        }
    }
    return output;
}
} // namespace Eden::Cheats
