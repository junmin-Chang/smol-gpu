#include "parser.hpp"
#include "instructions.hpp"
#include "lexer.hpp"
#include "token.hpp"

#define EXPECT_OR_RETURN(opt, type) \
    auto opt = expect<type>();     \
    if (!opt.has_value()) {        \
        return std::nullopt;       \
    }

namespace as {

namespace parser {

auto line_to_str(const parser::Line &line) -> std::string {
    return std::visit(overloaded{
                          [](const parser::BlocksDirective &blocks) { return std::format(".blocks {}", blocks.number); },
                          [](const parser::WarpsDirective &warps) { return std::format(".warps {}", warps.number); },
                          [](const parser::Instruction &instruction) {
                              std::string result;
                              if (instruction.label.has_value()) {
                                  result += std::format("{}: ", *instruction.label);
                              }
                              result += instruction.mnemonic.to_str();
                              // TODO: Add operands
                              return result;
                          },
                      },
                      line);
}

}

auto Parser::chop() -> std::optional<Token> {
    if (tokens.empty()) {
        return std::nullopt;
    }

    auto token = tokens.front();
    tokens = tokens.subspan(1);

    return token;
}

[[nodiscard]] auto Parser::peek() const -> const Token * {
    if (tokens.empty()) {
        return nullptr;
    }

    return &tokens.front();
}

void Parser::push_err(Error &&err) { errors.push_back(std::move(err)); }

void Parser::push_err(std::string &&message, unsigned column) {
    errors.push_back(Error{.message = std::move(message), .column = column});
}

void Parser::throw_unexpected_token(std::string &&expected, const Token &unexpected) {
    push_err(std::format("Unexpected token: Expected {}, instead found {}", expected, unexpected.to_str()), unexpected.col);
}

void Parser::throw_unexpected_eos(std::string &&expected) {
    push_err(std::format("Unexpected end of stream: Expected {}", expected), 0);
}

auto Parser::parse_instruction() -> std::optional<Result> {
    auto mnemonic_token = *chop();
    auto mnemonic = mnemonic_token.as<token::Mnemonic>().mnemonic;

    // HALT
    if (mnemonic.get_name() == sim::MnemonicName::HALT) {
        return parser::Instruction{.mnemonic = mnemonic};
    }

    // ADDI, SLTI, XORI, ORI, ANDI, SLLI, SRLI, SRAI
    /*if () {*/
    /*    return parse_itype_arithemtic_instruction(mnemonic);*/
    /*}*/

    /**/
    /*if (sim::is_of_type(mnemonic, sim::Opcode::RTYPE)) {*/
    /*    return parse_rtype_instruction(mnemonic);*/
    /*}*/
    /**/
    /*if (sim::is_of_type(mnemonic, sim::Opcode::STYPE)) {*/
    /*    return parse_stype_instruction(mnemonic);*/
    /*}*/

    push_err(std::format("Unknown mnemonic: '{}'", mnemonic.to_str()), mnemonic_token.col);
    return std::nullopt;
}

// <opcode> <rd>, <rs1>, <imm12>
auto Parser::parse_itype_arithemtic_instruction(sim::Mnemonic mnemonic) -> std::optional<parser::Instruction> {
    auto rd = expect<token::Register>();
    if (!rd.has_value()) {
        return std::nullopt;
    }

    EXPECT_OR_RETURN(comma1, token::Comma);
    EXPECT_OR_RETURN(rs1, token::Register);
    EXPECT_OR_RETURN(comma2, token::Comma);
    EXPECT_OR_RETURN(imm12, token::Immediate);

    auto instruction = parser::Instruction{
        .label = {},
        .mnemonic = mnemonic,
        /*.operands = {rd->as<token::Register>().number, rs1->as<token::Register>().number, imm12->as<Immediate>().value}*/
    };

    return parser::Instruction{.mnemonic = mnemonic};
}


auto Parser::parse_directive() -> std::optional<Result> {
    auto token = chop();
    if (!token) {
        return std::nullopt;
    }

    auto number = expect<token::Immediate>();
    if(!number.has_value()) {
        return std::nullopt;
    }

    const auto value = number->as<token::Immediate>().value;
    if (value < 1) {
        push_err(std::format("Invalid number of {}: '{}'", token->to_str(), value), number->col);
        return std::nullopt;
    }

    // The line should end here, otherwise it's an error
    if (peek() != nullptr) {
        throw_unexpected_token("End of line", *peek());
        return std::nullopt;
    }

    if (token->is_of_type<token::BlocksDirective>()) {
        return parser::BlocksDirective{.number = static_cast<std::uint32_t>(value)};
    } else {
        return parser::WarpsDirective{.number = static_cast<std::uint32_t>(value)};
    }

    return std::nullopt;
}

auto Parser::parse_line() -> std::optional<Result> {
    if (tokens.empty()) {
        return std::nullopt;
    }

    auto token = *peek();

    if (token.is_of_type<token::BlocksDirective>() || token.is_of_type<token::WarpsDirective>()) {
        return parse_directive();
    }

    std::optional<std::string> label = std::nullopt;

    if (token.is_of_type<token::Label>()) {
        label = token.as<token::Label>().name;
        chop();
    }

    if (token.is_of_type<token::Mnemonic>()) {
        auto instruction = parse_instruction();

        if(!instruction.has_value()) {
            return std::nullopt;
        }

        std::get<parser::Instruction>(instruction.value()).label = label;
        return parse_instruction();
    }

    push_err(std::format("Unexpected token: Expected end of line, instead found {}", token.to_str()), token.col);
    return std::nullopt;
}

auto parse_line(const std::string_view &json) -> std::expected<Parser::Result, std::vector<Parser::Error>> {
    auto [tokens, errors] = collect_tokens(json);
    if (!errors.empty()) {
        return std::unexpected(errors);
    }

    for (const auto &token : tokens) {
        std::println("{}", token.to_str());
    }

    Parser parser{tokens};
    auto result = parser.parse_line();
    if (!result.has_value()) {
        return std::unexpected(parser.consume_errors());
    }

    return result.value();
}

}
