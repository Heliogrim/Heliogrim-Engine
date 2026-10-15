#include "pch.h"

#include <Engine.Filesystem/Path.hpp>
#include <Engine.Filesystem/Url.hpp>

using namespace hg;

namespace FileSystemModule {
	using fs::Path;
	using fs::Url;

	/* Path */

	TEST(PathBasics, DefaultIsEmpty) {
		const Path path {};

		EXPECT_TRUE(path.empty());
		EXPECT_FALSE(path.hasName());
		EXPECT_EQ(path.segments(), 0u);
		EXPECT_TRUE(path.nativeName().empty());
	}

	TEST(PathBasics, NamedPathIsNotEmpty) {
		const Path path { "file.txt"sv };

		EXPECT_FALSE(path.empty());
		EXPECT_TRUE(path.hasName());
	}

	TEST(PathBasics, NameWithoutSeparatorIsWholePath) {
		EXPECT_EQ(Path { "file.txt"sv }.name(), "file.txt");
	}

	TEST(PathBasics, SegmentsCountSeparators) {
		EXPECT_EQ(Path { "a"sv }.segments(), 0u);
		EXPECT_EQ(Path { "a/b"sv }.normalized().segments(), 1u);
		EXPECT_EQ(Path { "a/b/c"sv }.normalized().segments(), 2u);
	}

	TEST(PathParent, HasParentOnlyWithSeparator) {
		EXPECT_FALSE(Path { "file.txt"sv }.hasParent());
		EXPECT_TRUE(Path { "dir/file.txt"sv }.hasParent());
		EXPECT_FALSE(Path {}.hasParent());
	}

	TEST(PathParent, ParentPathStripsLastSegment) {
		EXPECT_TRUE(Path { "a/b/c.txt"sv }.parentPath().equals(Path { "a/b"sv }));
	}

	TEST(PathParent, ParentNameIsNameOfParent) {
		const auto name = Path { "a/b/c.txt"sv }.parentName();

		EXPECT_NE(name.find('b'), string::npos);
	}

	TEST(PathMutation, PushAppendsSegment) {
		Path path { "a"sv };

		path.push(Path { "b"sv });

		EXPECT_EQ(path.segments(), 1u);
	}

	TEST(PathMutation, PopRemovesLastSegment) {
		Path path { "a/b/c"sv };

		path.pop();

		EXPECT_TRUE(path.equals(Path { "a/b"sv }));
	}

	TEST(PathMutation, PopCountRemovesMultipleSegments) {
		Path path { "a/b/c"sv };

		path.pop(2u);

		EXPECT_TRUE(path.equals(Path { "a"sv }));
	}

	TEST(PathMutation, PopZeroIsNoop) {
		Path path { "a/b"sv };

		path.pop(0u);

		EXPECT_TRUE(path.equals(Path { "a/b"sv }));
	}

	TEST(PathMutation, PopBeyondRootEndsEmpty) {
		Path path { "a"sv };

		path.pop(3u);

		EXPECT_TRUE(path.empty());
	}

	TEST(PathNormalize, CollapsesDotSegments) {
		EXPECT_TRUE(Path { "a/./b/../c"sv }.normalized().equals(Path { "a/c"sv }.normalized()));
	}

	TEST(PathCompare, EqualsIsLexical) {
		EXPECT_TRUE(Path { "a/b"sv }.equals(Path { "a/b"sv }));
		EXPECT_FALSE(Path { "a/b"sv }.equals(Path { "a/c"sv }));
		EXPECT_TRUE(Path { "a/b"sv } == Path { "a/b"sv });
	}

	TEST(PathCompare, LexicalOrdering) {
		EXPECT_TRUE(Path { "a"sv }.compareLexical(Path { "b"sv }) < 0);
		EXPECT_TRUE(Path { "b"sv }.compareLexical(Path { "a"sv }) > 0);
		EXPECT_TRUE(Path { "a"sv }.compareLexical(Path { "a"sv }) == 0);
	}

	TEST(PathContains, SiblingWithSharedPrefixIsNotContained) {
		const auto base = Path { "/x/dev/base"sv }.normalized();
		const auto sibling = Path { "/x/dev/base2/val"sv }.normalized();

		EXPECT_FALSE(base.contains(sibling));
		EXPECT_FALSE(sibling.isSubPathOf(base));
	}

	TEST(PathContains, BaseWithoutTrailingSeparatorContainsChild) {
		const auto base = Path { "/x/dev/base"sv }.normalized();
		const auto child = Path { "/x/dev/base/val"sv }.normalized();

		EXPECT_TRUE(base.contains(child));
		EXPECT_TRUE(child.isSubPathOf(base));
	}

	TEST(PathContains, EmptyBaseContainsNothingWithoutSeparator) {
		// An empty base is a degenerate parent: ensure it neither throws nor reports the base itself as contained.
		const Path empty {};

		EXPECT_FALSE(empty.contains(empty));
		EXPECT_FALSE(empty.isSubPathOf(empty));
	}

	TEST(PathContains, UnrelatedPathsAreNotContained) {
		const Path a { "/x/a/"sv };
		const Path b { "/y/b/val"sv };

		EXPECT_FALSE(a.contains(b));
		EXPECT_FALSE(b.isSubPathOf(a));
	}

	TEST(PathContains, ShorterCandidateIsNeverSubPath) {
		const Path deep { "/x/dev/base/sub"sv };
		const Path shallow { "/x/dev"sv };

		EXPECT_FALSE(shallow.isSubPathOf(deep));
		EXPECT_FALSE(shallow.contains(shallow));
	}

	/* Url */

	TEST(UrlTest, DefaultIsEmpty) {
		const Url url {};

		EXPECT_TRUE(url.empty());
		EXPECT_FALSE(url.hasScheme());
		EXPECT_FALSE(url.hasPath());
		EXPECT_EQ(url.encode(), "");
	}

	TEST(UrlTest, EncodeWithSchemePrefixesIt) {
		const Url url { "file"sv, Path { "a/b.txt"sv } };

		EXPECT_TRUE(url.hasScheme());
		EXPECT_TRUE(url.hasPath());
		EXPECT_EQ(url.scheme(), "file");
		EXPECT_EQ(url.encode(), "file://a/b.txt");
	}

	TEST(UrlTest, EncodeWithoutSchemeIsPathOnly) {
		const Url url { ""sv, Path { "a/b.txt"sv } };

		EXPECT_FALSE(url.hasScheme());
		EXPECT_EQ(url.encode(), "a/b.txt");
	}

	TEST(UrlTest, SettersReplaceParts) {
		Url url { "file"sv, Path { "a"sv } };

		url.setScheme("mem"sv).setPath(Path { "b"sv });

		EXPECT_EQ(url.scheme(), "mem");
		EXPECT_TRUE(url.path().equals(Path { "b"sv }));
		EXPECT_FALSE(url.empty());
	}

	TEST(UrlTest, SchemeOnlyIsNotEmpty) {
		Url url {};
		url.setScheme("file"sv);

		EXPECT_FALSE(url.empty());
		EXPECT_FALSE(url.hasPath());
	}

	TEST(UrlTest, ConvenienceConstructorForwardsPathString) {
		const Url url { "file"sv, "dir/x"sv };

		EXPECT_EQ(url.encode(), "file://dir/x");
	}
}

namespace FileSystemModule {
	// Separator handling is platform native and paths are not normalized on construction: `segments()` and
	// `contains()` only see the native separator. Documented here, as foreign separators silently yield 0 / false.
	TEST(PathContains, ForeignSeparatorsAreNotRecognizedWithoutNormalization) {
		const fs::Path base { "/x/dev/base"sv };
		const fs::Path child { "/x/dev/base/val"sv };

		#ifdef _WIN32
		EXPECT_EQ(fs::Path { "a/b"sv }.segments(), 0u);
		EXPECT_FALSE(base.contains(child));
		#else
		EXPECT_EQ(fs::Path { "a/b"sv }.segments(), 1u);
		EXPECT_TRUE(base.contains(child));
		#endif
	}
}
