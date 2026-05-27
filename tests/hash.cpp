#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <libdesktop.h>
#include <ranges>

using namespace desktop::helpers;

static std::filesystem::path write_temp_file(const std::string& name, std::string_view content)
{
	std::filesystem::path path{ std::filesystem::temp_directory_path() / name };
	std::ofstream{ path } << content;
	return path;
}

TEST(Hash, Sha256KnownEmptyString)
{
	ASSERT_EQ(hash::sha256_from_string(""), "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}

TEST(Hash, Sha256KnownValue)
{
	ASSERT_EQ(hash::sha256_from_string("hello"), "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824");
}

TEST(Hash, Sha256KnownValueLong)
{
	ASSERT_EQ(hash::sha256_from_string("The quick brown fox jumps over the lazy dog"), "d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592");
}

TEST(Hash, Sha256OutputLength)
{
	ASSERT_EQ(hash::sha256_from_string("any input").length(), 64U);
}

TEST(Hash, Sha256OutputIsLowerHex)
{
	std::string result{ hash::sha256_from_string("hello") };
	ASSERT_TRUE(std::ranges::all_of(result, [](char c)
	{
		return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
	}));
}

TEST(Hash, Sha256Deterministic)
{
	ASSERT_EQ(hash::sha256_from_string("hello"), hash::sha256_from_string("hello"));
}

TEST(Hash, Sha256DifferentInputsDifferentOutputs)
{
	ASSERT_NE(hash::sha256_from_string("hello"), hash::sha256_from_string("world"));
}

TEST(Hash, Sha256CaseSensitive)
{
	ASSERT_NE(hash::sha256_from_string("hello"), hash::sha256_from_string("Hello"));
}

TEST(Hash, Sha512KnownEmptyString)
{
	ASSERT_EQ(hash::sha512_from_string(""),
	          "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");
}

TEST(Hash, Sha512KnownValue)
{
	ASSERT_EQ(hash::sha512_from_string("hello"),
	          "9b71d224bd62f3785d96d46ad3ea3d73319bfbc2890caadae2dff72519673ca72323c3d99ba5c11d7c7acc6e14b8c5da0c4663475c2e5c3adef46f73bcdec043");
}

TEST(Hash, Sha512KnownValueLong)
{
	ASSERT_EQ(hash::sha512_from_string("The quick brown fox jumps over the lazy dog"),
	          "07e547d9586f6a73f73fbac0435ed76951218fb7d0c8d788a309d785436bbb642e93a252a954f23912547d1e8a3b5ed6e1bfd7097821233fa0538f3db854fee6");
}

TEST(Hash, Sha512OutputLength)
{
	ASSERT_EQ(hash::sha512_from_string("any input").length(), 128U);
}

TEST(Hash, Sha512OutputIsLowerHex)
{
	std::string result{ hash::sha512_from_string("hello") };
	ASSERT_TRUE(std::ranges::all_of(result, [](char c)
	{
		return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
	}));
}

TEST(Hash, Sha512Deterministic)
{
	ASSERT_EQ(hash::sha512_from_string("hello"), hash::sha512_from_string("hello"));
}

TEST(Hash, Sha512DifferentInputsDifferentOutputs)
{
	ASSERT_NE(hash::sha512_from_string("hello"), hash::sha512_from_string("world"));
}

TEST(Hash, Sha512CaseSensitive)
{
	ASSERT_NE(hash::sha512_from_string("hello"), hash::sha512_from_string("Hello"));
}

TEST(Hash, Sha256AndSha512DifferForSameInput)
{
	ASSERT_NE(hash::sha256_from_string("hello"), hash::sha512_from_string("hello"));
}

TEST(Hash, Sha256FileMatchesStringView)
{
	std::string content{ "hello from a file" };
	std::filesystem::path path{ write_temp_file("hash_test_sha256.txt", content) };
	ASSERT_EQ(hash::sha256_from_file(path), hash::sha256_from_string(content));
	std::filesystem::remove(path);
}

TEST(Hash, Sha256EmptyFile)
{
	std::filesystem::path path{ write_temp_file("hash_test_sha256_empty.txt", "") };
	ASSERT_EQ(hash::sha256_from_file(path), hash::sha256_from_string(""));
	std::filesystem::remove(path);
}

TEST(Hash, Sha256FileDeterministic)
{
	std::filesystem::path path{ write_temp_file("hash_test_sha256_det.txt", "repeat me") };
	ASSERT_EQ(hash::sha256_from_file(path), hash::sha256_from_file(path));
	std::filesystem::remove(path);
}

TEST(Hash, Sha256FileNotFound)
{
	ASSERT_EQ(hash::sha256_from_file(std::filesystem::path{ "/nonexistent/path/file.txt" }), "");
}

TEST(Hash, Sha512FileMatchesStringView)
{
	std::string content{ "hello from a file" };
	std::filesystem::path path{ write_temp_file("hash_test_sha512.txt", content) };
	ASSERT_EQ(hash::sha512_from_file(path), hash::sha512_from_string(content));
	std::filesystem::remove(path);
}

TEST(Hash, Sha512EmptyFile)
{
	std::filesystem::path path{ write_temp_file("hash_test_sha512_empty.txt", "") };
	ASSERT_EQ(hash::sha512_from_file(path), hash::sha512_from_string(""));
	std::filesystem::remove(path);
}

TEST(Hash, Sha512FileDeterministic)
{
	std::filesystem::path path{ write_temp_file("hash_test_sha512_det.txt", "repeat me") };
	ASSERT_EQ(hash::sha512_from_file(path), hash::sha512_from_file(path));
	std::filesystem::remove(path);
}

TEST(Hash, Sha512FileNotFound)
{
	ASSERT_EQ(hash::sha512_from_file(std::filesystem::path{ "/nonexistent/path/file.txt" }), "");
}