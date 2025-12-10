#include "fsm.hpp"
#include "fsm.h"

#include <cctype>

namespace sx {

void BufferFSM::on_declare(const BufferKey& key, std::size_t size, const SourceLocation& loc) {
    auto& info = buffers_[key];
    info.size = size;
    info.state = BufferState::Allocated;
    info.last_loc = loc;
}

void BufferFSM::on_write(const BufferKey& key, std::size_t bytes, const SourceLocation& loc) {
    auto it = buffers_.find(key);
    if (it == buffers_.end()) {
        // неизвестный буфер — заводим запись
        BufferInfo info;
        info.size = 0;
        info.state = BufferState::Unknown;
        info.last_loc = loc;
        it = buffers_.emplace(key, info).first;
    }
    it->second.last_loc = loc;

    // простая эвристика overflow'а: если размер известен и запись выходит за границы —
    // помечаем как Tainted
    if (it->second.size != 0 && bytes > it->second.size) {
        it->second.state = BufferState::Tainted;
    } else if (it->second.state == BufferState::Allocated || it->second.state == BufferState::Unknown) {
        it->second.state = BufferState::Initialized;
    }
}

void BufferFSM::on_read(const BufferKey& key, std::size_t /*bytes*/, const SourceLocation& loc) {
    auto it = buffers_.find(key);
    if (it == buffers_.end()) return;
    it->second.last_loc = loc;
}

void BufferFSM::on_sanitize(const BufferKey& key, const SourceLocation& loc) {
    auto it = buffers_.find(key);
    if (it == buffers_.end()) return;
    it->second.state = BufferState::Sanitized;
    it->second.last_loc = loc;
}

void BufferFSM::on_taint(const BufferKey& key, const SourceLocation& loc) {
    auto& info = buffers_[key];
    info.state = BufferState::Tainted;
    info.last_loc = loc;
}

BufferState BufferFSM::state_of(const BufferKey& key) const {
    auto it = buffers_.find(key);
    return it == buffers_.end() ? BufferState::Unknown : it->second.state;
}

std::vector<BufferFSM::BufferSnapshot> BufferFSM::snapshot() const {
    std::vector<BufferSnapshot> out;
    out.reserve(buffers_.size());
    for (const auto& kv : buffers_) {
        BufferSnapshot s;
        s.key      = kv.first;
        s.state    = kv.second.state;
        s.size     = kv.second.size;
        s.last_loc = kv.second.last_loc;
        out.push_back(std::move(s));
    }
    return out;
}

} // namespace sx

namespace sentinel {

std::vector<Token> tokenize(const std::string& line) {
    std::vector<Token> tokens;
    std::string current;
    std::size_t start_col = 0;

    enum class Mode { Normal, Identifier, Number };
    Mode mode = Mode::Normal;

    auto flush = [&]() {
        if (!current.empty()) {
            tokens.push_back(Token{current, start_col});
            current.clear();
        }
    };

    for (std::size_t i = 0; i < line.size(); ++i) {
        char c = line[i];

        switch (mode) {
        case Mode::Normal:
            if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
                mode = Mode::Identifier;
                start_col = i;
                current.push_back(c);
            } else if (std::isdigit(static_cast<unsigned char>(c))) {
                mode = Mode::Number;
                start_col = i;
                current.push_back(c);
            } else {
                // игнор
            }
            break;
        case Mode::Identifier:
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_') {
                current.push_back(c);
            } else {
                flush();
                mode = Mode::Normal;
            }
            break;
        case Mode::Number:
            if (std::isdigit(static_cast<unsigned char>(c))) {
                current.push_back(c);
            } else {
                flush();
                mode = Mode::Normal;
            }
            break;
        }
    }

    flush();
    return tokens;
}

SimpleFSM::SimpleFSM(TransitionCallback cb)
    : state_(State::Start), callback_(std::move(cb)) {}

void SimpleFSM::reset() {
    state_ = State::Start;
}

void SimpleFSM::process(const std::string& line) {
    for (std::size_t i = 0; i < line.size(); ++i) {
        char c    = line[i];
        char next = (i + 1 < line.size()) ? line[i + 1] : '\0';

        State prev = state_;
        State curr = state_;

        switch (state_) {
        case State::Start:
            if (c == '/' && next == '/') {
                curr = State::CommentLine;
            } else if (c == '/' && next == '*') {
                curr = State::CommentBlock;
            } else if (c == '"') {
                curr = State::StringLiteral;
            } else if (c == '\'') {
                curr = State::CharLiteral;
            } else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
                curr = State::Identifier;
            } else if (std::isdigit(static_cast<unsigned char>(c))) {
                curr = State::Number;
            } else {
                curr = State::Start;
            }
            break;

        case State::Identifier:
            if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_')) {
                curr = State::Start;
            }
            break;

        case State::Number:
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                curr = State::Start;
            }
            break;

        case State::StringLiteral:
            if (c == '\\' && next != '\0') {
                // just skip
            } else if (c == '"') {
                curr = State::Start;
            }
            break;

        case State::CharLiteral:
            if (c == '\\' && next != '\0') {
              // just skip
            } else if (c == '\'') {
                curr = State::Start;
            }
            break;

        case State::CommentLine:
            curr = State::CommentLine;
            break;

        case State::CommentBlock:
            if (c == '*' && next == '/') {
                curr = State::Start;
            } else {
                curr = State::CommentBlock;
            }
            break;
        }

        if (curr != state_) {
            if (callback_) {
                callback_(prev, curr, c, i);
            }
            state_ = curr;
        }
    }
}

} // namespace sentinel
