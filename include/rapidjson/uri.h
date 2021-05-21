// Tencent is pleased to support the open source community by making RapidJSON available.
// 
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip.
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

#ifndef RAPIDJSON_URI_H_
#define RAPIDJSON_URI_H_

#if defined(__clang__)
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(c++98-compat)
#elif defined(_MSC_VER)
RAPIDJSON_DIAG_OFF(4512) // assignment operator could not be generated
#endif

RAPIDJSON_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
// GenericUri

template <typename ValueType, typename Allocator=CrtAllocator>
class GenericUri {
public:
    typedef typename ValueType::Ch Ch;
    typedef std::basic_string<Ch> String;

    // Constructors
    GenericUri() : uri_(), base_(), scheme_(), auth_(), path_(), query_(), frag_() {}

    GenericUri(const String& uri) : uri_(), base_(), scheme_(), auth_(), path_(), query_(), frag_() {
        Parse(uri);
    }

    GenericUri(const Ch* uri, SizeType len) : uri_(), base_(), scheme_(), auth_(), path_(), query_(), frag_() {
        Parse(String(uri, len));
    }

    // Use with specializations of GenericValue
    template<typename T> GenericUri(const T& uri) : uri_(), base_(), scheme_(), auth_(), path_(), query_(), frag_() {
        Parse(uri.template Get<String>());
    }

    // Getters
    const String& Get() const { return uri_; }

    // Use with specializations of GenericValue
    template<typename T> void Get(T& uri, Allocator& allocator) {
        uri.template Set<String>(this->Get(), allocator);
    }

    const String& GetBase() const { return base_; }
    const String& GetScheme() const { return scheme_; }
    const String& GetAuth() const { return auth_; }
    const String& GetPath() const { return path_; }
    const String& GetQuery() const { return query_; }
    const String& GetFrag() const { return frag_; }

    const Ch* GetString() const { return uri_.c_str(); }
    SizeType GetStringLength() const { return static_cast<SizeType>(uri_.length()); }

    const Ch* GetBaseString() const { return base_.c_str(); }
    SizeType GetBaseStringLength() const { return static_cast<SizeType>(base_.length()); }

    const Ch* GetFragString() const { return frag_.c_str(); }
    SizeType GetFragStringLength() const { return static_cast<SizeType>(frag_.length()); }

    // Resolve this URI against another URI in accordance with URI resolution rules at
    // https://tools.ietf.org/html/rfc3986
    // Use for resolving an id or $ref with an in-scope id.
    // This URI is updated in place where needed from the base URI.
    GenericUri& Resolve(const GenericUri& uri) {
        if (!scheme_.empty()) {
            // Use all of this URI
            RemoveDotSegments(path_);
        } else {
            if (!auth_.empty()) {
                RemoveDotSegments(path_);
            } else {
                if (path_.empty()) {
                    path_ = uri.GetPath();
                    if (query_.empty()) {
                        query_ = uri.GetQuery();
                    }
                } else {
                    static const String slash = GetSlashString().GetString();
                    if (path_.find(slash) == 0) {
                        // Absolute path - replace all the path
                        RemoveDotSegments(path_);
                    } else {
                        // Relative path - append to path after last slash
                        String p;
                        if (!uri.GetAuth().empty() && uri.GetPath().empty()) p = slash;
                        std::size_t lastslashpos = uri.GetPath().find_last_of(slash);
                        path_ = p + uri.GetPath().substr(0, lastslashpos + 1) + path_;
                        RemoveDotSegments(path_);
                    }
                }
                auth_ = uri.GetAuth();
            }
            scheme_ = uri.GetScheme();
        }
        base_ = scheme_ + auth_ + path_ + query_;
        uri_ = base_ + frag_;
        return *this;
    }

    bool Match(const GenericUri& uri, bool full) const {
        if (full)
            return uri_ == uri.Get();
        else
            return base_ == uri.GetBase();
    }

  // Generate functions for string literal according to Ch
#define RAPIDJSON_STRING_(name, ...) \
    static const ValueType& Get##name##String() {\
        static const Ch s[] = { __VA_ARGS__, '\0' };\
        static const ValueType v(s, static_cast<SizeType>(sizeof(s) / sizeof(Ch) - 1));\
        return v;\
    }

    RAPIDJSON_STRING_(SchemeEnd, ':')
    RAPIDJSON_STRING_(AuthStart, '/', '/')
    RAPIDJSON_STRING_(QueryStart, '?')
    RAPIDJSON_STRING_(FragStart, '#')
    RAPIDJSON_STRING_(Slash, '/')
    RAPIDJSON_STRING_(Dot, '.')

#undef RAPIDJSON_STRING_

private:
    // Parse a URI into constituent scheme, authority, path, query, fragment
    // Supports URIs that match regex ^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))? as per
    // https://tools.ietf.org/html/rfc3986
    void Parse(const String& uri) {
        std::size_t start = 0, pos1 = 0, pos2 = 0;
        const std::size_t len = uri.length();
        static const String schemeEnd = GetSchemeEndString().GetString();
        static const String authStart = GetAuthStartString().GetString();
        static const String pathStart = GetSlashString().GetString();
        static const String queryStart = GetQueryStartString().GetString();
        static const String fragStart = GetFragStartString().GetString();
        // Look for scheme ([^:/?#]+):)?
        if (start < len) {
            pos1 = uri.find(schemeEnd);
            if (pos1 != std::string::npos) {
                pos2 = uri.find_first_of(pathStart + queryStart + fragStart);
                if (pos1 < pos2) {
                    pos1 += schemeEnd.length();
                    scheme_ = uri.substr(start, pos1);
                    start = pos1;
                }
            }
        }
        // Look for auth (//([^/?#]*))?
        if (start < len) {
            pos1 = uri.find(authStart, start);
            if (pos1 == start) {
                pos2 = uri.find_first_of(pathStart + queryStart + fragStart, start + authStart.length());
                auth_ = uri.substr(start, pos2 - start);
                start = pos2;
            }
        }
        // Look for path ([^?#]*)
        if (start < len) {
            pos2 = uri.find_first_of(queryStart + fragStart, start);
            if (start != pos2) {
                path_ = uri.substr(start, pos2 - start);
                if (path_.find(pathStart) == 0) {  // absolute path - normalize
                    RemoveDotSegments(path_);
                }
                start = pos2;
            }
        }
        // Look for query (\?([^#]*))?
        if (start < len) {
            pos2 = uri.find(fragStart, start);
            if (start != pos2) {
                query_ = uri.substr(start, pos2 - start);
                start = pos2;
            }
        }
        // Look for fragment (#(.*))?
        if (start < len) {
            frag_ = uri.substr(start);
        }
        base_ = scheme_ + auth_ + path_ + query_;
        uri_ = base_ + frag_;
    }

    // Remove . and .. segments from a path
    // https://tools.ietf.org/html/rfc3986
    void RemoveDotSegments(String& path) {
        String temp = path;
        path.clear();
        static const String slash = GetSlashString().GetString();
        static const String dot = GetDotString().GetString();
        std::size_t pos = 0;
        // Loop through each path segment
        while (pos != std::string::npos) {
            pos = temp.find_first_of(slash);
            // Get next segment
            String seg = temp.substr(0, pos);
            if (seg == dot) {
                // Discard . segment
            } else if (seg == dot + dot) {
                // Backup a .. segment
                // We expect to find a previously added slash at the end or nothing
                std::size_t pos1 = path.find_last_of(slash);
                // Make sure we don't go beyond the start
                if (pos1 != std::string::npos && pos1 != 0) {
                    // Find the next to last slash and back up to it
                    pos1 = path.find_last_of(slash, pos1 - 1);
                    path = path.substr(0, pos1 + 1);
                }
            } else {
                // Copy segment and add slash if not at end
                path += seg;
                if (pos != std::string::npos) path += slash;
            }
            // Move to next segment if not at end
            if (pos != std::string::npos) temp = temp.substr(pos + 1);
        }
    }

    String uri_;    // Full uri
    String base_;   // Everything except fragment
    String scheme_; // Includes the :
    String auth_;   // Includes the //
    String path_;   // Absolute if starts with /
    String query_;  // Includes the ?
    String frag_;   // Includes the #
};

//! GenericUri for Value (UTF-8, default allocator).
typedef GenericUri<Value> Uri;

RAPIDJSON_NAMESPACE_END

#if defined(__clang__)
RAPIDJSON_DIAG_POP
#endif

#endif // RAPIDJSON_URI_H_
