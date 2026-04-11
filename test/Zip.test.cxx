#include <gtest/gtest.h>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <DynXX/CXX/Zip.hxx>
#include <DynXX/CXX/Coding.hxx>
#include "TestUtil.hxx"

namespace {
    std::filesystem::path scriptFilePath() {
        return DynXX::TestUtil::resolveRepoRootPath() / "scripts" / "JS" / "DynXX.js";
    }

    std::string readFileAll(const std::filesystem::path &filePath) {
        std::ifstream in(filePath, std::ios::binary);
        return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    }
}

TEST(Zip, DynxxZZipInit) {
    const auto zip = dynxxZZipInit();
    ASSERT_NE(zip, 0U);
    dynxxZZipRelease(zip);
}

TEST(Zip, DynxxZZipInput) {
    const auto zip = dynxxZZipInit();
    ASSERT_NE(zip, 0U);
    const auto in = dynxxCodingStr2bytes("zip-input");
    EXPECT_EQ(dynxxZZipInput(zip, in, true), in.size());
    dynxxZZipRelease(zip);
}

TEST(Zip, DynxxZZipProcessDo) {
    const auto zip = dynxxZZipInit();
    ASSERT_NE(zip, 0U);
    const auto in = dynxxCodingStr2bytes("zip-process");
    ASSERT_EQ(dynxxZZipInput(zip, in, true), in.size());
    EXPECT_FALSE(dynxxZZipProcessDo(zip).empty());
    dynxxZZipRelease(zip);
}

TEST(Zip, DynxxZZipProcessFinished) {
    const auto zip = dynxxZZipInit();
    ASSERT_NE(zip, 0U);
    EXPECT_FALSE(dynxxZZipProcessFinished(zip));
    const auto in = dynxxCodingStr2bytes("zip-finished");
    ASSERT_EQ(dynxxZZipInput(zip, in, true), in.size());
    EXPECT_FALSE(dynxxZZipProcessDo(zip).empty());
    EXPECT_TRUE(dynxxZZipProcessFinished(zip));
    dynxxZZipRelease(zip);
}

TEST(Zip, DynxxZZipRelease) {
    const auto zip = dynxxZZipInit();
    ASSERT_NE(zip, 0U);
    EXPECT_NO_THROW(dynxxZZipRelease(zip));
}

TEST(Zip, DynxxZUnzipInit) {
    const auto unzip = dynxxZUnzipInit();
    ASSERT_NE(unzip, 0U);
    dynxxZUnzipRelease(unzip);
}

TEST(Zip, DynxxZUnzipInput) {
    const auto in = dynxxCodingStr2bytes("zip-unzip-input");
    const auto zipped = dynxxZBytesZip(in);
    ASSERT_FALSE(zipped.empty());
    const auto unzip = dynxxZUnzipInit();
    ASSERT_NE(unzip, 0U);
    EXPECT_EQ(dynxxZUnzipInput(unzip, zipped, true), zipped.size());
    dynxxZUnzipRelease(unzip);
}

TEST(Zip, DynxxZUnzipProcessDo) {
    const auto in = dynxxCodingStr2bytes("zip-unzip-process");
    const auto zipped = dynxxZBytesZip(in);
    ASSERT_FALSE(zipped.empty());
    const auto unzip = dynxxZUnzipInit();
    ASSERT_NE(unzip, 0U);
    ASSERT_EQ(dynxxZUnzipInput(unzip, zipped, true), zipped.size());
    EXPECT_EQ(dynxxZUnzipProcessDo(unzip), in);
    dynxxZUnzipRelease(unzip);
}

TEST(Zip, DynxxZUnzipProcessFinished) {
    const auto in = dynxxCodingStr2bytes("unzip-finished");
    const auto zipped = dynxxZBytesZip(in);
    ASSERT_FALSE(zipped.empty());
    const auto unzip = dynxxZUnzipInit();
    ASSERT_NE(unzip, 0U);
    EXPECT_FALSE(dynxxZUnzipProcessFinished(unzip));
    ASSERT_EQ(dynxxZUnzipInput(unzip, zipped, true), zipped.size());
    EXPECT_EQ(dynxxZUnzipProcessDo(unzip), in);
    EXPECT_TRUE(dynxxZUnzipProcessFinished(unzip));
    dynxxZUnzipRelease(unzip);
}

TEST(Zip, DynxxZUnzipRelease) {
    const auto unzip = dynxxZUnzipInit();
    ASSERT_NE(unzip, 0U);
    EXPECT_NO_THROW(dynxxZUnzipRelease(unzip));
}

TEST(Zip, DynxxZCFileZip) {
    const auto scriptPath = scriptFilePath();
    ASSERT_TRUE(std::filesystem::exists(scriptPath));
    const auto zipPath = std::filesystem::temp_directory_path() / "dynxx_cfile_zip.gz";
    std::FILE *in = std::fopen(scriptPath.string().c_str(), "rb");
    std::FILE *out = std::fopen(zipPath.string().c_str(), "wb");
    ASSERT_NE(in, nullptr);
    ASSERT_NE(out, nullptr);
    EXPECT_TRUE(dynxxZCFileZip(in, out, DynXXZipCompressModeX::Default, DynXXZDefaultBufferSize, DynXXZFormatX::GZip));
    std::fclose(in);
    std::fclose(out);
    EXPECT_TRUE(std::filesystem::exists(zipPath));
    EXPECT_GT(std::filesystem::file_size(zipPath), 0U);
}

TEST(Zip, DynxxZCFileUnzip) {
    const auto scriptPath = scriptFilePath();
    ASSERT_TRUE(std::filesystem::exists(scriptPath));
    const auto zipPath = std::filesystem::temp_directory_path() / "dynxx_cfile_unzip.gz";
    const auto unzipPath = std::filesystem::temp_directory_path() / "dynxx_cfile_unzip.js";
    std::FILE *zipIn = std::fopen(scriptPath.string().c_str(), "rb");
    std::FILE *zipOut = std::fopen(zipPath.string().c_str(), "wb");
    ASSERT_NE(zipIn, nullptr);
    ASSERT_NE(zipOut, nullptr);
    ASSERT_TRUE(dynxxZCFileZip(zipIn, zipOut, DynXXZipCompressModeX::Default, DynXXZDefaultBufferSize, DynXXZFormatX::GZip));
    std::fclose(zipIn);
    std::fclose(zipOut);
    std::FILE *unzipIn = std::fopen(zipPath.string().c_str(), "rb");
    std::FILE *unzipOut = std::fopen(unzipPath.string().c_str(), "wb");
    ASSERT_NE(unzipIn, nullptr);
    ASSERT_NE(unzipOut, nullptr);
    EXPECT_TRUE(dynxxZCFileUnzip(unzipIn, unzipOut, DynXXZDefaultBufferSize, DynXXZFormatX::GZip));
    std::fclose(unzipIn);
    std::fclose(unzipOut);
    EXPECT_EQ(readFileAll(scriptPath), readFileAll(unzipPath));
}

TEST(Zip, DynxxZCxxStreamZip) {
    const auto scriptPath = scriptFilePath();
    ASSERT_TRUE(std::filesystem::exists(scriptPath));
    std::ifstream in(scriptPath, std::ios::binary);
    ASSERT_TRUE(in.is_open());
    std::stringstream zipped;
    EXPECT_TRUE(dynxxZCxxStreamZip(&in, &zipped, DynXXZipCompressModeX::Default, DynXXZDefaultBufferSize, DynXXZFormatX::GZip));
    EXPECT_FALSE(zipped.str().empty());
}

TEST(Zip, DynxxZCxxStreamUnzip) {
    const auto scriptPath = scriptFilePath();
    ASSERT_TRUE(std::filesystem::exists(scriptPath));
    std::ifstream in(scriptPath, std::ios::binary);
    ASSERT_TRUE(in.is_open());
    std::stringstream zipped;
    ASSERT_TRUE(dynxxZCxxStreamZip(&in, &zipped, DynXXZipCompressModeX::Default, DynXXZDefaultBufferSize, DynXXZFormatX::GZip));
    std::istringstream zippedIn(zipped.str());
    std::stringstream out;
    EXPECT_TRUE(dynxxZCxxStreamUnzip(&zippedIn, &out, DynXXZDefaultBufferSize, DynXXZFormatX::GZip));
    EXPECT_EQ(out.str(), readFileAll(scriptPath));
}

TEST(Zip, DynxxZCxxStreamZipUnzipRoundTrip) {
    std::istringstream in("zip-stream-roundtrip");
    std::stringstream compressed;
    ASSERT_TRUE(dynxxZCxxStreamZip(&in, &compressed));

    std::istringstream compressedIn(compressed.str());
    std::stringstream out;
    ASSERT_TRUE(dynxxZCxxStreamUnzip(&compressedIn, &out));
    EXPECT_EQ(out.str(), "zip-stream-roundtrip");
}

TEST(Zip, DynxxZBytesZip) {
    const auto in = dynxxCodingStr2bytes("zip-bytes-roundtrip");
    const auto zipped = dynxxZBytesZip(in);
    ASSERT_FALSE(zipped.empty());
    EXPECT_EQ(dynxxZBytesUnzip(zipped), in);
}

TEST(Zip, DynxxZBytesUnzip) {
    const auto in = dynxxCodingStr2bytes("zip-bytes-roundtrip");
    const auto zipped = dynxxZBytesZip(in);
    ASSERT_FALSE(zipped.empty());
    EXPECT_EQ(dynxxZBytesUnzip(zipped), in);
}

TEST(Zip, DynxxZFileRoundTripWithScriptFile) {
    const auto scriptPath = scriptFilePath();
    ASSERT_TRUE(std::filesystem::exists(scriptPath));

    const auto zipPath = std::filesystem::temp_directory_path() / "dynxx_zip_script.gz";
    const auto unzipPath = std::filesystem::temp_directory_path() / "dynxx_unzip_script.js";

    std::ifstream in(scriptPath, std::ios::binary);
    std::ofstream zipped(zipPath, std::ios::binary | std::ios::trunc);
    ASSERT_TRUE(in.is_open());
    ASSERT_TRUE(zipped.is_open());
    ASSERT_TRUE(dynxxZCxxStreamZip(&in, &zipped, DynXXZipCompressModeX::Default, DynXXZDefaultBufferSize, DynXXZFormatX::GZip));
    in.close();
    zipped.close();

    std::ifstream zippedIn(zipPath, std::ios::binary);
    std::ofstream out(unzipPath, std::ios::binary | std::ios::trunc);
    ASSERT_TRUE(zippedIn.is_open());
    ASSERT_TRUE(out.is_open());
    ASSERT_TRUE(dynxxZCxxStreamUnzip(&zippedIn, &out, DynXXZDefaultBufferSize, DynXXZFormatX::GZip));
    zippedIn.close();
    out.close();

    EXPECT_EQ(readFileAll(scriptPath), readFileAll(unzipPath));
}
