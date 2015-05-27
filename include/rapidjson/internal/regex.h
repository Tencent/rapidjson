// Tencent is pleased to support the open source community by making RapidJSON available.
// 
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip. All rights reserved.
//
// Licensed under the MIT License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, software distributed 
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
// CONDITIONS OF ANY KIND, either express or implied. See the License for the 
// specific language governing permissions and limitations under the License.

#ifndef RAPIDJSON_INTERNAL_REGEX_H_
#define RAPIDJSON_INTERNAL_REGEX_H_

#include "../rapidjson.h"
#include "stack.h"

#ifndef RAPIDJSON_REGEX_VERBOSE
#define RAPIDJSON_REGEX_VERBOSE 0
#endif

RAPIDJSON_NAMESPACE_BEGIN
namespace internal {

///////////////////////////////////////////////////////////////////////////////
// GenericRegex

static const SizeType kRegexInvalidState = ~SizeType(0);  //!< Represents an invalid index in GenericRegex::State::out, out1
static const SizeType kRegexInvalidRange = ~SizeType(0);

//! Regular expression engine with subset of ECMAscript grammar.
/*!
    Supported regular expression syntax:
    - \c ab     Concatenation
    - \c a|b    Alternation
    - \c a?     Zero or one
    - \c a*     Zero or more
    - \c a+     One or more
    - \c a{3}   Exactly 3 times
    - \c a{3,}  At least 3 times
    - \c a{3,5} 3 to 5 times
    - \c (ab)*  Grouping
    - \c .      Any character
    - \c [abc]  Character classes
    - \c [a-c]  Character class range
    - \c [a-z0-9_] Character class combination
    - \c [^abc] Negated character classes
    - \c [^a-c] Negated character class range
    - \c [\b]   Backspace (U+0008)
    - \c \\| \\\\ ...  Escape characters
    - \c \\f Form feed (U+000C)
    - \c \\n Line feed (U+000A)
    - \c \\r Carriage return (U+000D)
    - \c \\t Tab (U+0009)
    - \c \\v Vertical tab (U+000B)
*/
template <typename Encoding, typename Allocator = CrtAllocator>
class GenericRegex {
public:
    typedef typename Encoding::Ch Ch;

    GenericRegex(const Ch* source, Allocator* allocator = 0) : states_(allocator, 256), ranges_(allocator, 256), root_(kRegexInvalidState), stateCount_(),rangeCount_() {
        StringStream ss(source);
        DecodedStream<StringStream> ds(ss);
        Parse(ds);
    }

    ~GenericRegex() {
    }

    bool IsValid() const {
        return root_ != kRegexInvalidState;
    }

    template <typename InputStream>
    bool Match(InputStream& is) const {
        RAPIDJSON_ASSERT(IsValid());
        DecodedStream<InputStream> ds(is);

        Allocator allocator;
        Stack<Allocator> state0(&allocator, stateCount_ * sizeof(SizeType));
        Stack<Allocator> state1(&allocator, stateCount_ * sizeof(SizeType));
        Stack<Allocator> *current = &state0, *next = &state1;

        const size_t stateSetSize = (stateCount_ + 31) / 32 * 4;
        unsigned* stateSet = static_cast<unsigned*>(allocator.Malloc(stateSetSize));
        std::memset(stateSet, 0, stateSetSize);
        AddState(stateSet, *current, root_);

        unsigned codepoint;
        while (!current->Empty() && (codepoint = ds.Take()) != 0) {
            std::memset(stateSet, 0, stateSetSize);
            next->Clear();
            for (const SizeType* s = current->template Bottom<SizeType>(); s != current->template End<SizeType>(); ++s) {
                const State& sr = GetState(*s);
                if (sr.codepoint == codepoint ||
                    sr.codepoint == kAnyCharacterClass || 
                    (sr.codepoint == kRangeCharacterClass && MatchRange(sr.rangeStart, codepoint)))
                {
                    AddState(stateSet, *next, sr.out);
                }
            }
            Stack<Allocator>* temp = current;
            current = next;
            next = temp;
        }

        Allocator::Free(stateSet);

        for (const SizeType* s = current->template Bottom<SizeType>(); s != current->template End<SizeType>(); ++s)
            if (GetState(*s).out == kRegexInvalidState)
                return true;

        return false;
    }

    bool Match(const Ch* s) {
        StringStream is(s);
        return Match(is);
    }

private:
    enum Operator {
        kZeroOrOne,
        kZeroOrMore,
        kOneOrMore,
        kConcatenation,
        kAlternation,
        kLeftParenthesis
    };

    static const unsigned kAnyCharacterClass = 0xFFFFFFFF;   //!< For '.'
    static const unsigned kRangeCharacterClass = 0xFFFFFFFE;
    static const unsigned kRangeNegationFlag = 0x80000000;

    struct Range {
        unsigned start; // 
        unsigned end;
        SizeType next;
    };

    struct State {
        SizeType out;     //!< Equals to kInvalid for matching state
        SizeType out1;    //!< Equals to non-kInvalid for split
        SizeType rangeStart;
        unsigned codepoint;
    };

    struct Frag {
        Frag(SizeType s, SizeType o) : start(s), out(o) {}
        SizeType start;
        SizeType out; //!< link-list of all output states
    };

    template <typename SourceStream>
    class DecodedStream {
    public:
        DecodedStream(SourceStream& ss) : ss_(ss) { Decode(); }
        unsigned Peek() { return codepoint_; }
        unsigned Take() { unsigned c = codepoint_; Decode(); return c; }

    private:
        void Decode() {
            if (!Encoding::Decode(ss_, &codepoint_))
                codepoint_ = 0;
        }

        SourceStream& ss_;
        unsigned codepoint_;
    };

    State& GetState(SizeType index) {
        RAPIDJSON_ASSERT(index < stateCount_);
        return states_.template Bottom<State>()[index];
    }

    const State& GetState(SizeType index) const {
        RAPIDJSON_ASSERT(index < stateCount_);
        return states_.template Bottom<State>()[index];
    }

    Range& GetRange(SizeType index) {
        RAPIDJSON_ASSERT(index < rangeCount_);
        return ranges_.template Bottom<Range>()[index];
    }

    const Range& GetRange(SizeType index) const {
        RAPIDJSON_ASSERT(index < rangeCount_);
        return ranges_.template Bottom<Range>()[index];
    }

    void AddState(unsigned* stateSet, Stack<Allocator>& l, SizeType index) const {
        if (index == kRegexInvalidState)
            return;

        const State& s = GetState(index);
        if (s.out1 != kRegexInvalidState) { // Split
            AddState(stateSet, l, s.out);
            AddState(stateSet, l, s.out1);
        }
        else if (!(stateSet[index >> 5] & (1 << (index & 31)))) {
            stateSet[index >> 5] |= (1 << (index & 31));
            *l.template Push<SizeType>() = index;
        }
    }

    bool MatchRange(SizeType rangeIndex, unsigned codepoint) const {
        bool yes = (GetRange(rangeIndex).start & kRangeNegationFlag) == 0;
        while (rangeIndex != kRegexInvalidRange) {
            const Range& r = GetRange(rangeIndex);
            if (codepoint >= (r.start & ~kRangeNegationFlag) && codepoint <= r.end)
                return yes;
            rangeIndex = r.next;
        }
        return !yes;
    }

    template <typename InputStream>
    void Parse(DecodedStream<InputStream>& ds) {
        Allocator allocator;
        Stack<Allocator> operandStack(&allocator, 256);     // Frag
        Stack<Allocator> operatorStack(&allocator, 256);    // Operator
        Stack<Allocator> atomCountStack(&allocator, 256);   // unsigned (Atom per parenthesis)

        *atomCountStack.template Push<unsigned>() = 0;

        unsigned codepoint;
        while (ds.Peek() != 0) {
            switch (codepoint = ds.Take()) {
                case '|':
                    while (!operatorStack.Empty() && *operatorStack.template Top<Operator>() < kAlternation)
                        if (!Eval(operandStack, *operatorStack.template Pop<Operator>(1)))
                            return;
                    *operatorStack.template Push<Operator>() = kAlternation;
                    *atomCountStack.template Top<unsigned>() = 0;
                    break;

                case '(':
                    *operatorStack.template Push<Operator>() = kLeftParenthesis;
                    *atomCountStack.template Push<unsigned>() = 0;
                    break;

                case ')':
                    while (!operatorStack.Empty() && *operatorStack.template Top<Operator>() != kLeftParenthesis)
                        if (!Eval(operandStack, *operatorStack.template Pop<Operator>(1)))
                            return;
                    if (operatorStack.Empty())
                        return;
                    operatorStack.template Pop<Operator>(1);
                    atomCountStack.template Pop<unsigned>(1);
                    ImplicitConcatenation(atomCountStack, operatorStack);
                    break;

                case '?':
                    if (!Eval(operandStack, kZeroOrOne))
                        return;
                    break;

                case '*':
                    if (!Eval(operandStack, kZeroOrMore))
                        return;
                    break;

                case '+':
                    if (!Eval(operandStack, kOneOrMore))
                        return;
                    break;

                case '{':
                    {
                        unsigned n, m;
                        if (!ParseUnsigned(ds, &n) || n == 0)
                            return;

                        if (ds.Peek() == ',') {
                            ds.Take();
                            if (ds.Peek() == '}')
                                m = 0;
                            else if (!ParseUnsigned(ds, &m) || m < n)
                                return;
                        }
                        else
                            m = n;

                        if (!EvalQuantifier(operandStack, n, m) || ds.Peek() != '}')
                            return;
                        ds.Take();
                    }
                    break;

                case '.':
                    PushOperand(operandStack, kAnyCharacterClass);
                    ImplicitConcatenation(atomCountStack, operatorStack);
                    break;

                case '[':
                    {
                        SizeType range;
                        if (!ParseRange(ds, &range))
                            return;
                        SizeType s = NewState(kRegexInvalidState, kRegexInvalidState, kRangeCharacterClass);
                        GetState(s).rangeStart = range;
                        *operandStack.template Push<Frag>() = Frag(s, s);
                    }
                    ImplicitConcatenation(atomCountStack, operatorStack);
                    break;

                case '\\': // Escape character
                    if (!CharacterEscape(ds, &codepoint))
                        return; // Unsupported escape character
                    // fall through to default

                default: // Pattern character
                    PushOperand(operandStack, codepoint);
                    ImplicitConcatenation(atomCountStack, operatorStack);
            }
        }

        while (!operatorStack.Empty())
            if (!Eval(operandStack, *operatorStack.template Pop<Operator>(1)))
                return;

        // Link the operand to matching state.
        if (operandStack.GetSize() == sizeof(Frag)) {
            Frag* e = operandStack.template Pop<Frag>(1);
            Patch(e->out, NewState(kRegexInvalidState, kRegexInvalidState, 0));
            root_ = e->start;

#if RAPIDJSON_REGEX_VERBOSE
            printf("root: %d\n", root_);
            for (SizeType i = 0; i < stateCount_ ; i++) {
                State& s = GetState(i);
                printf("[%2d] out: %2d out1: %2d c: '%c'\n", i, s.out, s.out1, (char)s.codepoint);
            }
            printf("\n");
#endif
        }
    }

    SizeType NewState(SizeType out, SizeType out1, unsigned codepoint) {
        State* s = states_.template Push<State>();
        s->out = out;
        s->out1 = out1;
        s->codepoint = codepoint;
        s->rangeStart = kRegexInvalidRange;
        return stateCount_++;
    }

    void PushOperand(Stack<Allocator>& operandStack, unsigned codepoint) {
        SizeType s = NewState(kRegexInvalidState, kRegexInvalidState, codepoint);
        *operandStack.template Push<Frag>() = Frag(s, s);
    }

    void ImplicitConcatenation(Stack<Allocator>& atomCountStack, Stack<Allocator>& operatorStack) {
        if (*atomCountStack.template Top<unsigned>())
            *operatorStack.template Push<Operator>() = kConcatenation;
        (*atomCountStack.template Top<unsigned>())++;
    }

    SizeType Append(SizeType l1, SizeType l2) {
        SizeType old = l1;
        while (GetState(l1).out != kRegexInvalidState)
            l1 = GetState(l1).out;
        GetState(l1).out = l2;
        return old;
    }

    void Patch(SizeType l, SizeType s) {
        for (SizeType next; l != kRegexInvalidState; l = next) {
            next = GetState(l).out;
            GetState(l).out = s;
        }
    }

    bool Eval(Stack<Allocator>& operandStack, Operator op) {
        switch (op) {
            case kConcatenation:
                if (operandStack.GetSize() >= sizeof(Frag) * 2) {
                    Frag e2 = *operandStack.template Pop<Frag>(1);
                    Frag e1 = *operandStack.template Pop<Frag>(1);
                    Patch(e1.out, e2.start);
                    *operandStack.template Push<Frag>() = Frag(e1.start, e2.out);
                    return true;
                }
                return false;

            case kAlternation:
                if (operandStack.GetSize() >= sizeof(Frag) * 2) {
                    Frag e2 = *operandStack.template Pop<Frag>(1);
                    Frag e1 = *operandStack.template Pop<Frag>(1);
                    SizeType s = NewState(e1.start, e2.start, 0);
                    *operandStack.template Push<Frag>() = Frag(s, Append(e1.out, e2.out));
                    return true;
                }
                return false;

            case kZeroOrOne:
                if (operandStack.GetSize() >= sizeof(Frag)) {
                    Frag e = *operandStack.template Pop<Frag>(1);
                    SizeType s = NewState(kRegexInvalidState, e.start, 0);
                    *operandStack.template Push<Frag>() = Frag(s, Append(e.out, s));
                    return true;
                }
                return false;

            case kZeroOrMore:
                if (operandStack.GetSize() >= sizeof(Frag)) {
                    Frag e = *operandStack.template Pop<Frag>(1);
                    SizeType s = NewState(kRegexInvalidState, e.start, 0);
                    Patch(e.out, s);
                    *operandStack.template Push<Frag>() = Frag(s, s);
                    return true;
                }
                return false;

            case kOneOrMore:
                if (operandStack.GetSize() >= sizeof(Frag)) {
                    Frag e = *operandStack.template Pop<Frag>(1);
                    SizeType s = NewState(kRegexInvalidState, e.start, 0);
                    Patch(e.out, s);
                    *operandStack.template Push<Frag>() = Frag(e.start, s);
                    return true;
                }
                return false;

            default:
                return false;
        }
    }

    bool EvalQuantifier(Stack<Allocator>& operandStack, unsigned n, unsigned m) {
        RAPIDJSON_ASSERT(n > 0);
        RAPIDJSON_ASSERT(m == 0 || n <= m);         // m == 0 means infinity
        if (operandStack.GetSize() < sizeof(Frag))
            return false;

        for (unsigned i = 0; i < n - 1; i++)        // a{3} -> a a a
            CloneTopOperand(operandStack);

        if (m == 0)
            Eval(operandStack, kOneOrMore);         // a{3,} -> a a a+
        else if (m > n) {
            CloneTopOperand(operandStack);          // a{3,5} -> a a a a
            Eval(operandStack, kZeroOrOne);         // a{3,5} -> a a a a?
            for (unsigned i = n; i < m - 1; i++)
                CloneTopOperand(operandStack);      // a{3,5} -> a a a a? a?
            for (unsigned i = n; i < m; i++)
                Eval(operandStack, kConcatenation); // a{3,5} -> a a aa?a?
        }

        for (unsigned i = 0; i < n - 1; i++)
            Eval(operandStack, kConcatenation);     // a{3} -> aaa, a{3,} -> aaa+, a{3.5} -> aaaa?a?

        return true;
    }

    static SizeType Min(SizeType a, SizeType b) { return a < b ? a : b; }

    SizeType GetMinStateIndex(SizeType index) {
        State& s = GetState(index);
        if (s.out != kRegexInvalidState && s.out < index)
            index = Min(index, GetMinStateIndex(s.out));
        if (s.out1 != kRegexInvalidState && s.out1 < index)
            index = Min(index, GetMinStateIndex(s.out1));
        return index;
    }

    void CloneTopOperand(Stack<Allocator>& operandStack) {
        const Frag *src = operandStack.template Top<Frag>();
        SizeType minIndex = GetMinStateIndex(src->start);
        SizeType count = stateCount_ - minIndex; // Assumes top operand contains states in [min, stateCount_)
        State* s = states_.template Push<State>(count);
        memcpy(s, &GetState(minIndex), count * sizeof(State));
        for (SizeType j = 0; j < count; j++) {
            if (s[j].out != kRegexInvalidState)
                s[j].out += count;
            if (s[j].out1 != kRegexInvalidState)
                s[j].out1 += count;
        }
        *operandStack.template Push<Frag>() = Frag(src->start + count, src->out + count);
        stateCount_ += count;
    }

    template <typename InputStream>
    bool ParseUnsigned(DecodedStream<InputStream>& ds, unsigned* u) {
        unsigned r = 0;
        while (ds.Peek() >= '0' && ds.Peek() <= '9') {
            if (r >= 429496729 && ds.Peek() > '5') // 2^32 - 1 = 4294967295
                return false; // overflow
            r = r * 10 + (ds.Take() - '0');
        }
        *u = r;
        return true;
    }

    template <typename InputStream>
    bool ParseRange(DecodedStream<InputStream>& ds, SizeType* range) {
        bool isBegin = true;
        bool negate = false;
        int step = 0;
        SizeType start = kRegexInvalidRange;
        SizeType current = kRegexInvalidRange;
        unsigned codepoint;
        while ((codepoint = ds.Take()) != 0) {
            if (isBegin) {
                isBegin = false;
                if (codepoint == '^') {
                    negate = true;
                    continue;
                }
            }

            switch (codepoint) {
            case ']':
                if (step == 2) { // Add trailing '-'
                    SizeType r = NewRange('-');
                    RAPIDJSON_ASSERT(current != kRegexInvalidRange);
                    GetRange(current).next = r;
                }
                if (negate)
                    GetRange(start).start |= kRangeNegationFlag;
                *range = start;
                return true;

            case '\\':
                if (ds.Peek() == 'b') {
                    ds.Take();
                    codepoint = 0x0008; // Escape backspace character
                }
                else if (!CharacterEscape(ds, &codepoint))
                    return false;
                // fall through to default

            default:
                switch (step) {
                case 1:
                    if (codepoint == '-') {
                        step++;
                        break;
                    }
                    // fall through to step 0 for other characters

                case 0:
                    {
                        SizeType r = NewRange(codepoint);
                        if (current != kRegexInvalidRange)
                            GetRange(current).next = r;
                        if (start == kRegexInvalidRange)
                            start = r;
                        current = r;
                    }
                    step = 1;
                    break;

                default:
                    RAPIDJSON_ASSERT(step == 2);
                    GetRange(current).end = codepoint;
                    step = 0;
                }
            }
        }
        return false;
    }
    
    SizeType NewRange(unsigned codepoint) {
        Range* r = ranges_.template Push<Range>();
        r->start = r->end = codepoint;
        r->next = kRegexInvalidRange;
        return rangeCount_++;
    }

    template <typename InputStream>
    bool CharacterEscape(DecodedStream<InputStream>& ds, unsigned* escapedCodepoint) {
        unsigned codepoint;
        switch (codepoint = ds.Take()) {
            case '|':
            case '(':
            case ')':
            case '?':
            case '*':
            case '+':
            case '.':
            case '[':
            case ']':
            case '{':
            case '}':
            case '\\':
                *escapedCodepoint = codepoint; return true;
            case 'f': *escapedCodepoint = 0x000C; return true;
            case 'n': *escapedCodepoint = 0x000A; return true;
            case 'r': *escapedCodepoint = 0x000D; return true;
            case 't': *escapedCodepoint = 0x0009; return true;
            case 'v': *escapedCodepoint = 0x000B; return true;
            default:
                return false; // Unsupported escape character
        }
    }

    Stack<Allocator> states_;
    Stack<Allocator> ranges_;
    SizeType root_;
    SizeType stateCount_;
    SizeType rangeCount_;
};

typedef GenericRegex<UTF8<> > Regex;

} // namespace internal
RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_INTERNAL_REGEX_H_
