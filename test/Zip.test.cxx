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

    struct ZipModeFormatCase {
        DynXXZipCompressModeX mode;
        DynXXZFormatX format;
    };
}

TEST(Zip, DynxxZZipInit) {
    const auto zip = dynxxZZipInit();
    ASSERT_NE(zip, 0U);
    dynxxZZipRelease(zip);
}

TEST(Zip, DynxxZZipInit_InvalidBufferSize) {
    EXPECT_EQ(dynxxZZipInit(DynXXZipCompressModeX::Default, 0, DynXXZFormatX::ZLib), 0U);
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

TEST(Zip, DynxxZZipHandleInvalid) {
    const auto in = dynxxCodingStr2bytes("zip-invalid-handle");
    EXPECT_EQ(dynxxZZipInput(0U, in, true), 0U);
    EXPECT_TRUE(dynxxZZipProcessDo(0U).empty());
    EXPECT_FALSE(dynxxZZipProcessFinished(0U));
    EXPECT_NO_THROW(dynxxZZipRelease(0U));

    const auto zip = dynxxZZipInit();
    ASSERT_NE(zip, 0U);
    dynxxZZipRelease(zip);
    EXPECT_EQ(dynxxZZipInput(zip, in, true), 0U);
    EXPECT_TRUE(dynxxZZipProcessDo(zip).empty());
    EXPECT_FALSE(dynxxZZipProcessFinished(zip));
}

TEST(Zip, DynxxZUnzipInit) {
    const auto unzip = dynxxZUnzipInit();
    ASSERT_NE(unzip, 0U);
    dynxxZUnzipRelease(unzip);
}

TEST(Zip, DynxxZUnzipInit_InvalidBufferSize) {
    EXPECT_EQ(dynxxZUnzipInit(0, DynXXZFormatX::ZLib), 0U);
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

TEST(Zip, DynxxZUnzipHandleInvalid) {
    const auto in = dynxxCodingStr2bytes("unzip-invalid-handle");
    EXPECT_EQ(dynxxZUnzipInput(0U, in, true), 0U);
    EXPECT_TRUE(dynxxZUnzipProcessDo(0U).empty());
    EXPECT_FALSE(dynxxZUnzipProcessFinished(0U));
    EXPECT_NO_THROW(dynxxZUnzipRelease(0U));

    const auto unzip = dynxxZUnzipInit();
    ASSERT_NE(unzip, 0U);
    dynxxZUnzipRelease(unzip);
    EXPECT_EQ(dynxxZUnzipInput(unzip, in, true), 0U);
    EXPECT_TRUE(dynxxZUnzipProcessDo(unzip).empty());
    EXPECT_FALSE(dynxxZUnzipProcessFinished(unzip));
}

TEST(Zip, DynxxZCFileZip) {
    const auto scriptPath = scriptFilePath();
    ASSERT_TRUE(std::filesystem::exists(scriptPath));
    const auto zipPath = DynXX::TestUtil::resolveTempPath() / "dynxx_cfile_zip.gz";
    std::FILE *in = DynXX::TestUtil::openFile(scriptPath, "rb");
    std::FILE *out = DynXX::TestUtil::openFile(zipPath, "wb");
    ASSERT_NE(in, nullptr);
    ASSERT_NE(out, nullptr);
    EXPECT_TRUE(dynxxZCFileZip(in, out, DynXXZipCompressModeX::Default, DynXXZDefaultBufferSize, DynXXZFormatX::GZip));
    std::fclose(in);
    std::fclose(out);
    EXPECT_TRUE(std::filesystem::exists(zipPath));
    EXPECT_GT(std::filesystem::file_size(zipPath), 0U);
}

TEST(Zip, DynxxZCFileZip_InvalidParams) {
    EXPECT_FALSE(dynxxZCFileZip(nullptr, nullptr));
    EXPECT_FALSE(dynxxZCFileZip(nullptr, nullptr, DynXXZipCompressModeX::Default, 0, DynXXZFormatX::ZLib));

    const auto outPath = DynXX::TestUtil::resolveTempPath() / "dynxx_cfile_zip_null_in.gz";
    std::FILE *out = DynXX::TestUtil::openFile(outPath, "wb");
    ASSERT_NE(out, nullptr);
    EXPECT_FALSE(dynxxZCFileZip(nullptr, out));
    std::fclose(out);

    const auto inPath = scriptFilePath();
    std::FILE *in = DynXX::TestUtil::openFile(inPath, "rb");
    ASSERT_NE(in, nullptr);
    EXPECT_FALSE(dynxxZCFileZip(in, nullptr));
    std::fclose(in);
}

TEST(Zip, DynxxZCFileZip_FileNotExists) {
    const auto notExistsPath = DynXX::TestUtil::resolveTempPath() / "dynxx_not_exists_zip_in.js";
    std::filesystem::remove(notExistsPath);
    std::FILE *in = DynXX::TestUtil::openFile(notExistsPath, "rb");
    ASSERT_EQ(in, nullptr);

    const auto outPath = DynXX::TestUtil::resolveTempPath() / "dynxx_cfile_zip_not_exists.gz";
    std::FILE *out = DynXX::TestUtil::openFile(outPath, "wb");
    ASSERT_NE(out, nullptr);
    EXPECT_FALSE(dynxxZCFileZip(in, out));
    std::fclose(out);
}

TEST(Zip, DynxxZCFileUnzip) {
    const auto scriptPath = scriptFilePath();
    ASSERT_TRUE(std::filesystem::exists(scriptPath));
    const auto zipPath = DynXX::TestUtil::resolveTempPath() / "dynxx_cfile_unzip.gz";
    const auto unzipPath = DynXX::TestUtil::resolveTempPath() / "dynxx_cfile_unzip.js";
    std::FILE *zipIn = DynXX::TestUtil::openFile(scriptPath, "rb");
    std::FILE *zipOut = DynXX::TestUtil::openFile(zipPath, "wb");
    ASSERT_NE(zipIn, nullptr);
    ASSERT_NE(zipOut, nullptr);
    ASSERT_TRUE(dynxxZCFileZip(zipIn, zipOut, DynXXZipCompressModeX::Default, DynXXZDefaultBufferSize, DynXXZFormatX::GZip));
    std::fclose(zipIn);
    std::fclose(zipOut);
    std::FILE *unzipIn = DynXX::TestUtil::openFile(zipPath, "rb");
    std::FILE *unzipOut = DynXX::TestUtil::openFile(unzipPath, "wb");
    ASSERT_NE(unzipIn, nullptr);
    ASSERT_NE(unzipOut, nullptr);
    EXPECT_TRUE(dynxxZCFileUnzip(unzipIn, unzipOut, DynXXZDefaultBufferSize, DynXXZFormatX::GZip));
    std::fclose(unzipIn);
    std::fclose(unzipOut);
    EXPECT_EQ(readFileAll(scriptPath), readFileAll(unzipPath));
}

TEST(Zip, DynxxZCFileUnzip_InvalidParams) {
    EXPECT_FALSE(dynxxZCFileUnzip(nullptr, nullptr));
    EXPECT_FALSE(dynxxZCFileUnzip(nullptr, nullptr, 0, DynXXZFormatX::ZLib));

    const auto outPath = DynXX::TestUtil::resolveTempPath() / "dynxx_cfile_unzip_null_in.js";
    std::FILE *out = DynXX::TestUtil::openFile(outPath, "wb");
    ASSERT_NE(out, nullptr);
    EXPECT_FALSE(dynxxZCFileUnzip(nullptr, out));
    std::fclose(out);

    const auto inPath = scriptFilePath();
    std::FILE *in = DynXX::TestUtil::openFile(inPath, "rb");
    ASSERT_NE(in, nullptr);
    EXPECT_FALSE(dynxxZCFileUnzip(in, nullptr));
    std::fclose(in);
}

TEST(Zip, DynxxZCFileUnzip_FileNotExists) {
    const auto notExistsPath = DynXX::TestUtil::resolveTempPath() / "dynxx_not_exists_unzip_in.gz";
    std::filesystem::remove(notExistsPath);
    std::FILE *in = DynXX::TestUtil::openFile(notExistsPath, "rb");
    ASSERT_EQ(in, nullptr);

    const auto outPath = DynXX::TestUtil::resolveTempPath() / "dynxx_cfile_unzip_not_exists.js";
    std::FILE *out = DynXX::TestUtil::openFile(outPath, "wb");
    ASSERT_NE(out, nullptr);
    EXPECT_FALSE(dynxxZCFileUnzip(in, out));
    std::fclose(out);
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

TEST(Zip, DynxxZCxxStreamZip_InvalidParams) {
    EXPECT_FALSE(dynxxZCxxStreamZip(nullptr, nullptr));
    EXPECT_FALSE(dynxxZCxxStreamZip(nullptr, nullptr, DynXXZipCompressModeX::Default, 0, DynXXZFormatX::ZLib));

    std::stringstream out;
    EXPECT_FALSE(dynxxZCxxStreamZip(nullptr, &out));

    std::istringstream in("zip-stream-null-out");
    EXPECT_FALSE(dynxxZCxxStreamZip(&in, nullptr));
}

TEST(Zip, DynxxZCxxStreamZip_FileNotExists) {
    const auto notExistsPath = DynXX::TestUtil::resolveTempPath() / "dynxx_not_exists_stream_zip_in.js";
    std::filesystem::remove(notExistsPath);
    std::ifstream in(notExistsPath, std::ios::binary);
    ASSERT_FALSE(in.is_open());
    std::stringstream out;
    EXPECT_FALSE(dynxxZCxxStreamZip(&in, &out));
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

TEST(Zip, DynxxZCxxStreamUnzip_InvalidParams) {
    EXPECT_FALSE(dynxxZCxxStreamUnzip(nullptr, nullptr));
    EXPECT_FALSE(dynxxZCxxStreamUnzip(nullptr, nullptr, 0, DynXXZFormatX::ZLib));

    std::stringstream out;
    EXPECT_FALSE(dynxxZCxxStreamUnzip(nullptr, &out));

    std::istringstream in("unzip-stream-null-out");
    EXPECT_FALSE(dynxxZCxxStreamUnzip(&in, nullptr));
}

TEST(Zip, DynxxZCxxStreamUnzip_FileNotExists) {
    const auto notExistsPath = DynXX::TestUtil::resolveTempPath() / "dynxx_not_exists_stream_unzip_in.gz";
    std::filesystem::remove(notExistsPath);
    std::ifstream in(notExistsPath, std::ios::binary);
    ASSERT_FALSE(in.is_open());
    std::stringstream out;
    EXPECT_FALSE(dynxxZCxxStreamUnzip(&in, &out));
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

TEST(Zip, DynxxZBytesZip_InvalidParams) {
    EXPECT_TRUE(dynxxZBytesZip({}).empty());
    EXPECT_TRUE(dynxxZBytesZip(dynxxCodingStr2bytes("abc"), DynXXZipCompressModeX::Default, 0, DynXXZFormatX::ZLib).empty());
}

TEST(Zip, DynxxZBytesUnzip) {
    const auto in = dynxxCodingStr2bytes("zip-bytes-roundtrip");
    const auto zipped = dynxxZBytesZip(in);
    ASSERT_FALSE(zipped.empty());
    EXPECT_EQ(dynxxZBytesUnzip(zipped), in);
}

TEST(Zip, DynxxZBytesUnzip_InvalidParams) {
    EXPECT_TRUE(dynxxZBytesUnzip({}).empty());
    EXPECT_TRUE(dynxxZBytesUnzip(dynxxCodingStr2bytes("abc"), 0, DynXXZFormatX::ZLib).empty());
}

TEST(Zip, DynxxZFileRoundTripWithScriptFile) {
    const auto scriptPath = scriptFilePath();
    ASSERT_TRUE(std::filesystem::exists(scriptPath));

    const auto zipPath = DynXX::TestUtil::resolveTempPath() / "dynxx_zip_script.gz";
    const auto unzipPath = DynXX::TestUtil::resolveTempPath() / "dynxx_unzip_script.js";

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

class ZipModeFormatMatrixTest : public ::testing::TestWithParam<ZipModeFormatCase> {};

TEST_P(ZipModeFormatMatrixTest, BytesZipUnzipRoundTrip) {
    const auto param = GetParam();
    const auto in = dynxxCodingStr2bytes("zip-matrix-roundtrip-data-1234567890");
    const auto zipped = dynxxZBytesZip(in, param.mode, DynXXZDefaultBufferSize, param.format);
    ASSERT_FALSE(zipped.empty());
    EXPECT_EQ(dynxxZBytesUnzip(zipped, DynXXZDefaultBufferSize, param.format), in);
}

TEST_P(ZipModeFormatMatrixTest, StreamZipUnzipRoundTrip) {
    const auto param = GetParam();
    std::istringstream in("zip-stream-matrix-roundtrip");
    std::stringstream compressed;
    ASSERT_TRUE(dynxxZCxxStreamZip(&in, &compressed, param.mode, DynXXZDefaultBufferSize, param.format));

    std::istringstream compressedIn(compressed.str());
    std::stringstream out;
    ASSERT_TRUE(dynxxZCxxStreamUnzip(&compressedIn, &out, DynXXZDefaultBufferSize, param.format));
    EXPECT_EQ(out.str(), "zip-stream-matrix-roundtrip");
}

INSTANTIATE_TEST_SUITE_P(
    AllModeAndFormat,
    ZipModeFormatMatrixTest,
    ::testing::Values(
        ZipModeFormatCase{DynXXZipCompressModeX::Default, DynXXZFormatX::ZLib},
        ZipModeFormatCase{DynXXZipCompressModeX::Default, DynXXZFormatX::GZip},
        ZipModeFormatCase{DynXXZipCompressModeX::Default, DynXXZFormatX::Raw},
        ZipModeFormatCase{DynXXZipCompressModeX::PreferSpeed, DynXXZFormatX::ZLib},
        ZipModeFormatCase{DynXXZipCompressModeX::PreferSpeed, DynXXZFormatX::GZip},
        ZipModeFormatCase{DynXXZipCompressModeX::PreferSpeed, DynXXZFormatX::Raw},
        ZipModeFormatCase{DynXXZipCompressModeX::PreferSize, DynXXZFormatX::ZLib},
        ZipModeFormatCase{DynXXZipCompressModeX::PreferSize, DynXXZFormatX::GZip},
        ZipModeFormatCase{DynXXZipCompressModeX::PreferSize, DynXXZFormatX::Raw}
    )
);
