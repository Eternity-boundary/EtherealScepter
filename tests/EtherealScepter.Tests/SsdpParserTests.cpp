//Created by: EternityBoundary
// Unit tests for SsdpParser
#include "pch.h"

// Include the header directly
#include "../../include/Services/SsdpParser.h"

using namespace EtherealScepter::Services::Upnp;

// ===========================================
// ParseSsdpHeaders Tests
// ===========================================

TEST(SsdpParserTests, ParseSsdpHeaders_ValidNotifyResponse_ReturnsAllHeaders)
{
    std::string ssdpResponse =
        "NOTIFY * HTTP/1.1\r\n"
        "HOST: 239.255.255.250:1900\r\n"
        "LOCATION: http://192.168.1.1:5000/rootDesc.xml\r\n"
        "SERVER: Linux/3.14 UPnP/1.0 miniupnpd/1.9\r\n"
        "NT: urn:schemas-upnp-org:service:WANIPConnection:1\r\n"
        "USN: uuid:abc123::urn:schemas-upnp-org:service:WANIPConnection:1\r\n"
        "NTS: ssdp:alive\r\n"
        "\r\n";

    SsdpHeaders headers = ParseSsdpHeaders(ssdpResponse);

    EXPECT_EQ("http://192.168.1.1:5000/rootDesc.xml", headers.location);
    EXPECT_EQ("Linux/3.14 UPnP/1.0 miniupnpd/1.9", headers.server);
    EXPECT_EQ("urn:schemas-upnp-org:service:WANIPConnection:1", headers.nt);
    EXPECT_EQ("uuid:abc123::urn:schemas-upnp-org:service:WANIPConnection:1", headers.usn);
    EXPECT_EQ("ssdp:alive", headers.nts);
}

TEST(SsdpParserTests, ParseSsdpHeaders_MSearchResponse_ReturnsLocationAndST)
{
    std::string ssdpResponse =
        "HTTP/1.1 200 OK\r\n"
        "LOCATION: http://192.168.0.1:49152/gatedesc.xml\r\n"
        "ST: urn:schemas-upnp-org:device:InternetGatewayDevice:1\r\n"
        "SERVER: Windows/10.0 UPnP/1.1 MiniUPnPd/2.0\r\n"
        "USN: uuid:device-uuid::urn:schemas-upnp-org:device:InternetGatewayDevice:1\r\n"
        "\r\n";

    SsdpHeaders headers = ParseSsdpHeaders(ssdpResponse);

    EXPECT_EQ("http://192.168.0.1:49152/gatedesc.xml", headers.location);
    EXPECT_EQ("urn:schemas-upnp-org:device:InternetGatewayDevice:1", headers.st);
    EXPECT_EQ("Windows/10.0 UPnP/1.1 MiniUPnPd/2.0", headers.server);
}

TEST(SsdpParserTests, ParseSsdpHeaders_CaseInsensitive_ParsesCorrectly)
{
    std::string ssdpResponse =
        "HTTP/1.1 200 OK\r\n"
        "LOCATION: http://192.168.1.1:80/description.xml\r\n"
        "server: Apache/2.4\r\n"
        "St: upnp:rootdevice\r\n"
        "\r\n";

    SsdpHeaders headers = ParseSsdpHeaders(ssdpResponse);

    EXPECT_EQ("http://192.168.1.1:80/description.xml", headers.location);
    EXPECT_EQ("Apache/2.4", headers.server);
    EXPECT_EQ("upnp:rootdevice", headers.st);
}

TEST(SsdpParserTests, ParseSsdpHeaders_EmptyResponse_ReturnsEmptyHeaders)
{
    std::string ssdpResponse = "";

    SsdpHeaders headers = ParseSsdpHeaders(ssdpResponse);

    EXPECT_TRUE(headers.location.empty());
    EXPECT_TRUE(headers.server.empty());
    EXPECT_TRUE(headers.st.empty());
    EXPECT_TRUE(headers.nt.empty());
    EXPECT_TRUE(headers.usn.empty());
    EXPECT_TRUE(headers.nts.empty());
}

TEST(SsdpParserTests, ParseSsdpHeaders_MissingHeaders_ReturnsPartialResults)
{
    std::string ssdpResponse =
        "HTTP/1.1 200 OK\r\n"
        "LOCATION: http://10.0.0.1/desc.xml\r\n"
        "\r\n";

    SsdpHeaders headers = ParseSsdpHeaders(ssdpResponse);

    EXPECT_EQ("http://10.0.0.1/desc.xml", headers.location);
    EXPECT_TRUE(headers.server.empty());
    EXPECT_TRUE(headers.st.empty());
}

TEST(SsdpParserTests, ParseSsdpHeaders_HeaderWithLeadingSpaces_TrimsValue)
{
    std::string ssdpResponse =
        "HTTP/1.1 200 OK\r\n"
        "LOCATION:    http://192.168.1.1/desc.xml\r\n"
        "SERVER:  \t Linux UPnP/1.0\r\n"
        "\r\n";

    SsdpHeaders headers = ParseSsdpHeaders(ssdpResponse);

    EXPECT_EQ("http://192.168.1.1/desc.xml", headers.location);
    EXPECT_EQ("Linux UPnP/1.0", headers.server);
}

TEST(SsdpParserTests, ParseSsdpHeaders_IPv6Location_ParsesCorrectly)
{
    std::string ssdpResponse =
        "HTTP/1.1 200 OK\r\n"
        "LOCATION: http://[fe80::1]:5000/rootDesc.xml\r\n"
        "\r\n";

    SsdpHeaders headers = ParseSsdpHeaders(ssdpResponse);

    EXPECT_EQ("http://[fe80::1]:5000/rootDesc.xml", headers.location);
}

// ===========================================
// TryGetLocation Tests
// ===========================================

TEST(SsdpParserTests, TryGetLocation_ValidResponse_ReturnsLocation)
{
    std::string ssdpResponse =
        "HTTP/1.1 200 OK\r\n"
        "LOCATION: http://192.168.1.1:5000/rootDesc.xml\r\n"
        "\r\n";

    auto result = TryGetLocation(ssdpResponse);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("http://192.168.1.1:5000/rootDesc.xml", result.value());
}

TEST(SsdpParserTests, TryGetLocation_NoLocationHeader_ReturnsNullopt)
{
    std::string ssdpResponse =
        "HTTP/1.1 200 OK\r\n"
        "SERVER: Linux UPnP/1.0\r\n"
        "\r\n";

    auto result = TryGetLocation(ssdpResponse);

    EXPECT_FALSE(result.has_value());
}

TEST(SsdpParserTests, TryGetLocation_EmptyResponse_ReturnsNullopt)
{
    std::string ssdpResponse = "";

    auto result = TryGetLocation(ssdpResponse);

    EXPECT_FALSE(result.has_value());
}

TEST(SsdpParserTests, TryGetLocation_MalformedResponse_ReturnsNullopt)
{
    std::string ssdpResponse = "LOCATION http://invalid";

    auto result = TryGetLocation(ssdpResponse);

    EXPECT_FALSE(result.has_value());
}

TEST(SsdpParserTests, TryGetLocation_LowerCaseHeader_ParsesCorrectly)
{
    std::string ssdpResponse =
        "HTTP/1.1 200 OK\r\n"
        "location: http://192.168.0.1:8080/igd.xml\r\n"
        "\r\n";

    auto result = TryGetLocation(ssdpResponse);

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("http://192.168.0.1:8080/igd.xml", result.value());
}
