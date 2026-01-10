//Created by: EternityBoundary
// Unit tests for IgdDescriptionParser helper functions
#include "pch.h"

namespace IgdTestHelpers
{
    static std::wstring BytesToWideUtf8OrAcp(const std::string& bytes)
    {
        if (bytes.empty())
            return L"";

        int needed = MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            bytes.data(),
            (int)bytes.size(),
            nullptr,
            0);

        if (needed > 0)
        {
            std::wstring w;
            w.resize(needed);
            MultiByteToWideChar(
                CP_UTF8,
                MB_ERR_INVALID_CHARS,
                bytes.data(),
                (int)bytes.size(),
                w.data(),
                needed);
            return w;
        }

        needed = MultiByteToWideChar(
            CP_ACP,
            0,
            bytes.data(),
            (int)bytes.size(),
            nullptr,
            0);

        if (needed <= 0)
            return L"";

        std::wstring w;
        w.resize(needed);
        MultiByteToWideChar(
            CP_ACP,
            0,
            bytes.data(),
            (int)bytes.size(),
            w.data(),
            needed);
        return w;
    }

    static void SanitizeXmlInPlace(std::wstring& xml)
    {
        if (xml.empty())
            return;

        if (!xml.empty() && xml.front() == 0xFEFF)
            xml.erase(xml.begin());

        auto pos = xml.find(L'<');
        if (pos != std::wstring::npos && pos > 0)
            xml.erase(0, pos);

        xml.erase(std::remove(xml.begin(), xml.end(), L'\0'), xml.end());
    }

    static std::wstring ExtractBaseUrl(const std::wstring& location)
    {
        auto slash = location.find(L'/', location.find(L"//") + 2);
        return (slash == std::wstring::npos)
            ? location
            : location.substr(0, slash);
    }
}

// ===========================================
// BytesToWideUtf8OrAcp Tests
// ===========================================

TEST(IgdDescriptionParserHelperTests, BytesToWide_EmptyString_ReturnsEmpty)
{
    std::string input = "";
    auto result = IgdTestHelpers::BytesToWideUtf8OrAcp(input);
    EXPECT_TRUE(result.empty());
}

TEST(IgdDescriptionParserHelperTests, BytesToWide_AsciiString_ConvertsCorrectly)
{
    std::string input = "Hello World";
    auto result = IgdTestHelpers::BytesToWideUtf8OrAcp(input);
    EXPECT_EQ(std::wstring(L"Hello World"), result);
}

TEST(IgdDescriptionParserHelperTests, BytesToWide_Utf8Chinese_ConvertsCorrectly)
{
    std::string input = "\xE6\xB8\xAC\xE8\xA9\xA6";
    auto result = IgdTestHelpers::BytesToWideUtf8OrAcp(input);
    EXPECT_EQ(std::wstring(L"測試"), result);
}

TEST(IgdDescriptionParserHelperTests, BytesToWide_XmlContent_ConvertsCorrectly)
{
    std::string input = "<?xml version=\"1.0\"?>";
    auto result = IgdTestHelpers::BytesToWideUtf8OrAcp(input);
    EXPECT_EQ(std::wstring(L"<?xml version=\"1.0\"?>"), result);
}

// ===========================================
// SanitizeXmlInPlace Tests
// ===========================================

TEST(IgdDescriptionParserHelperTests, SanitizeXml_EmptyString_RemainsEmpty)
{
    std::wstring xml = L"";
    IgdTestHelpers::SanitizeXmlInPlace(xml);
    EXPECT_TRUE(xml.empty());
}

TEST(IgdDescriptionParserHelperTests, SanitizeXml_ValidXml_Unchanged)
{
    std::wstring xml = L"<?xml version=\"1.0\"?><root></root>";
    IgdTestHelpers::SanitizeXmlInPlace(xml);
    EXPECT_EQ(std::wstring(L"<?xml version=\"1.0\"?><root></root>"), xml);
}

TEST(IgdDescriptionParserHelperTests, SanitizeXml_WithBom_RemovesBom)
{
    std::wstring xml = L"\xFEFF<?xml version=\"1.0\"?>";
    IgdTestHelpers::SanitizeXmlInPlace(xml);
    EXPECT_EQ(std::wstring(L"<?xml version=\"1.0\"?>"), xml);
}

TEST(IgdDescriptionParserHelperTests, SanitizeXml_LeadingGarbage_RemovesGarbage)
{
    std::wstring xml = L"   garbage<root></root>";
    IgdTestHelpers::SanitizeXmlInPlace(xml);
    EXPECT_EQ(std::wstring(L"<root></root>"), xml);
}

TEST(IgdDescriptionParserHelperTests, SanitizeXml_EmbeddedNulls_RemovesNulls)
{
    std::wstring xml = std::wstring(L"<root>") + std::wstring(1, L'\0') + std::wstring(L"test</root>");
    IgdTestHelpers::SanitizeXmlInPlace(xml);
    EXPECT_EQ(std::wstring(L"<root>test</root>"), xml);
}

TEST(IgdDescriptionParserHelperTests, SanitizeXml_BomAndLeadingGarbage_RemovesBoth)
{
    std::wstring xml = L"\xFEFF  junk<root/>";
    IgdTestHelpers::SanitizeXmlInPlace(xml);
    EXPECT_EQ(std::wstring(L"<root/>"), xml);
}

// ===========================================
// ExtractBaseUrl Tests
// ===========================================

TEST(IgdDescriptionParserHelperTests, ExtractBaseUrl_StandardUrl_ReturnsSchemeHostPort)
{
    std::wstring location = L"http://192.168.1.1:5000/rootDesc.xml";
    auto result = IgdTestHelpers::ExtractBaseUrl(location);
    EXPECT_EQ(std::wstring(L"http://192.168.1.1:5000"), result);
}

TEST(IgdDescriptionParserHelperTests, ExtractBaseUrl_NoPath_ReturnsFullUrl)
{
    std::wstring location = L"http://192.168.1.1:8080";
    auto result = IgdTestHelpers::ExtractBaseUrl(location);
    EXPECT_EQ(std::wstring(L"http://192.168.1.1:8080"), result);
}

TEST(IgdDescriptionParserHelperTests, ExtractBaseUrl_DefaultPort_ReturnsCorrectBase)
{
    std::wstring location = L"http://192.168.0.1/igd.xml";
    auto result = IgdTestHelpers::ExtractBaseUrl(location);
    EXPECT_EQ(std::wstring(L"http://192.168.0.1"), result);
}

TEST(IgdDescriptionParserHelperTests, ExtractBaseUrl_HttpsUrl_ReturnsCorrectBase)
{
    std::wstring location = L"https://192.168.1.1:443/upnp/IGD.xml";
    auto result = IgdTestHelpers::ExtractBaseUrl(location);
    EXPECT_EQ(std::wstring(L"https://192.168.1.1:443"), result);
}

TEST(IgdDescriptionParserHelperTests, ExtractBaseUrl_DeepPath_ReturnsOnlyBase)
{
    std::wstring location = L"http://10.0.0.1:49152/upnp/control/WANIPConn1";
    auto result = IgdTestHelpers::ExtractBaseUrl(location);
    EXPECT_EQ(std::wstring(L"http://10.0.0.1:49152"), result);
}

TEST(IgdDescriptionParserHelperTests, ExtractBaseUrl_IPv6Address_ReturnsCorrectBase)
{
    std::wstring location = L"http://[fe80::1]:5000/rootDesc.xml";
    auto result = IgdTestHelpers::ExtractBaseUrl(location);
    EXPECT_EQ(std::wstring(L"http://[fe80::1]:5000"), result);
}
