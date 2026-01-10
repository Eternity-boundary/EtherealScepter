//Created by: EternityBoundary
// Unit tests for PortPageViewModel logic
#include "pch.h"

struct TestPortMappingInfo
{
    std::wstring Description;
    uint16_t ExternalPort;
    uint16_t InternalPort;
    std::wstring Protocol;
    std::wstring InternalClient;

    bool operator==(const TestPortMappingInfo& other) const
    {
        return Description == other.Description &&
            ExternalPort == other.ExternalPort &&
            InternalPort == other.InternalPort &&
            Protocol == other.Protocol &&
            InternalClient == other.InternalClient;
    }
};

namespace PortMappingLogic
{
    bool MappingsMatch(const TestPortMappingInfo& a, const TestPortMappingInfo& b)
    {
        return a.Description == b.Description &&
            a.ExternalPort == b.ExternalPort &&
            a.InternalPort == b.InternalPort &&
            a.Protocol == b.Protocol &&
            a.InternalClient == b.InternalClient;
    }

    bool TryRemoveMapping(std::vector<TestPortMappingInfo>& mappings, const TestPortMappingInfo& toRemove)
    {
        for (auto it = mappings.begin(); it != mappings.end(); ++it)
        {
            if (MappingsMatch(*it, toRemove))
            {
                mappings.erase(it);
                return true;
            }
        }
        return false;
    }
}

// ===========================================
// MappingsMatch Tests
// ===========================================

TEST(PortPageViewModelTests, MappingsMatch_IdenticalMappings_ReturnsTrue)
{
    TestPortMappingInfo a{ L"Web Server", 80, 80, L"TCP", L"192.168.1.100" };
    TestPortMappingInfo b{ L"Web Server", 80, 80, L"TCP", L"192.168.1.100" };

    EXPECT_TRUE(PortMappingLogic::MappingsMatch(a, b));
}

TEST(PortPageViewModelTests, MappingsMatch_DifferentDescription_ReturnsFalse)
{
    TestPortMappingInfo a{ L"Web Server", 80, 80, L"TCP", L"192.168.1.100" };
    TestPortMappingInfo b{ L"HTTP Server", 80, 80, L"TCP", L"192.168.1.100" };

    EXPECT_FALSE(PortMappingLogic::MappingsMatch(a, b));
}

TEST(PortPageViewModelTests, MappingsMatch_DifferentExternalPort_ReturnsFalse)
{
    TestPortMappingInfo a{ L"Web Server", 80, 80, L"TCP", L"192.168.1.100" };
    TestPortMappingInfo b{ L"Web Server", 8080, 80, L"TCP", L"192.168.1.100" };

    EXPECT_FALSE(PortMappingLogic::MappingsMatch(a, b));
}

TEST(PortPageViewModelTests, MappingsMatch_DifferentInternalPort_ReturnsFalse)
{
    TestPortMappingInfo a{ L"Web Server", 80, 80, L"TCP", L"192.168.1.100" };
    TestPortMappingInfo b{ L"Web Server", 80, 8080, L"TCP", L"192.168.1.100" };

    EXPECT_FALSE(PortMappingLogic::MappingsMatch(a, b));
}

TEST(PortPageViewModelTests, MappingsMatch_DifferentProtocol_ReturnsFalse)
{
    TestPortMappingInfo a{ L"DNS Server", 53, 53, L"TCP", L"192.168.1.100" };
    TestPortMappingInfo b{ L"DNS Server", 53, 53, L"UDP", L"192.168.1.100" };

    EXPECT_FALSE(PortMappingLogic::MappingsMatch(a, b));
}

TEST(PortPageViewModelTests, MappingsMatch_DifferentInternalClient_ReturnsFalse)
{
    TestPortMappingInfo a{ L"Web Server", 80, 80, L"TCP", L"192.168.1.100" };
    TestPortMappingInfo b{ L"Web Server", 80, 80, L"TCP", L"192.168.1.101" };

    EXPECT_FALSE(PortMappingLogic::MappingsMatch(a, b));
}

TEST(PortPageViewModelTests, MappingsMatch_EmptyDescriptions_ReturnsTrue)
{
    TestPortMappingInfo a{ L"", 80, 80, L"TCP", L"192.168.1.100" };
    TestPortMappingInfo b{ L"", 80, 80, L"TCP", L"192.168.1.100" };

    EXPECT_TRUE(PortMappingLogic::MappingsMatch(a, b));
}

// ===========================================
// TryRemoveMapping Tests
// ===========================================

TEST(PortPageViewModelTests, TryRemoveMapping_ExistingMapping_RemovesAndReturnsTrue)
{
    std::vector<TestPortMappingInfo> mappings = {
        { L"Web Server", 80, 80, L"TCP", L"192.168.1.100" },
        { L"SSH", 22, 22, L"TCP", L"192.168.1.100" },
        { L"FTP", 21, 21, L"TCP", L"192.168.1.100" }
    };

    TestPortMappingInfo toRemove{ L"SSH", 22, 22, L"TCP", L"192.168.1.100" };

    bool result = PortMappingLogic::TryRemoveMapping(mappings, toRemove);

    EXPECT_TRUE(result);
    EXPECT_EQ(size_t(2), mappings.size());
}

TEST(PortPageViewModelTests, TryRemoveMapping_NonExistingMapping_ReturnsFalse)
{
    std::vector<TestPortMappingInfo> mappings = {
        { L"Web Server", 80, 80, L"TCP", L"192.168.1.100" },
        { L"SSH", 22, 22, L"TCP", L"192.168.1.100" }
    };

    TestPortMappingInfo toRemove{ L"FTP", 21, 21, L"TCP", L"192.168.1.100" };

    bool result = PortMappingLogic::TryRemoveMapping(mappings, toRemove);

    EXPECT_FALSE(result);
    EXPECT_EQ(size_t(2), mappings.size());
}

TEST(PortPageViewModelTests, TryRemoveMapping_EmptyVector_ReturnsFalse)
{
    std::vector<TestPortMappingInfo> mappings;

    TestPortMappingInfo toRemove{ L"Web Server", 80, 80, L"TCP", L"192.168.1.100" };

    bool result = PortMappingLogic::TryRemoveMapping(mappings, toRemove);

    EXPECT_FALSE(result);
    EXPECT_TRUE(mappings.empty());
}

TEST(PortPageViewModelTests, TryRemoveMapping_FirstElement_RemovesCorrectly)
{
    std::vector<TestPortMappingInfo> mappings = {
        { L"Web Server", 80, 80, L"TCP", L"192.168.1.100" },
        { L"SSH", 22, 22, L"TCP", L"192.168.1.100" }
    };

    TestPortMappingInfo toRemove{ L"Web Server", 80, 80, L"TCP", L"192.168.1.100" };

    bool result = PortMappingLogic::TryRemoveMapping(mappings, toRemove);

    EXPECT_TRUE(result);
    EXPECT_EQ(size_t(1), mappings.size());
    EXPECT_EQ(std::wstring(L"SSH"), mappings[0].Description);
}

TEST(PortPageViewModelTests, TryRemoveMapping_LastElement_RemovesCorrectly)
{
    std::vector<TestPortMappingInfo> mappings = {
        { L"Web Server", 80, 80, L"TCP", L"192.168.1.100" },
        { L"SSH", 22, 22, L"TCP", L"192.168.1.100" }
    };

    TestPortMappingInfo toRemove{ L"SSH", 22, 22, L"TCP", L"192.168.1.100" };

    bool result = PortMappingLogic::TryRemoveMapping(mappings, toRemove);

    EXPECT_TRUE(result);
    EXPECT_EQ(size_t(1), mappings.size());
    EXPECT_EQ(std::wstring(L"Web Server"), mappings[0].Description);
}

TEST(PortPageViewModelTests, TryRemoveMapping_DuplicateMappings_RemovesOnlyFirst)
{
    std::vector<TestPortMappingInfo> mappings = {
        { L"Web Server", 80, 80, L"TCP", L"192.168.1.100" },
        { L"Web Server", 80, 80, L"TCP", L"192.168.1.100" },
        { L"SSH", 22, 22, L"TCP", L"192.168.1.100" }
    };

    TestPortMappingInfo toRemove{ L"Web Server", 80, 80, L"TCP", L"192.168.1.100" };

    bool result = PortMappingLogic::TryRemoveMapping(mappings, toRemove);

    EXPECT_TRUE(result);
    EXPECT_EQ(size_t(2), mappings.size());
    EXPECT_EQ(std::wstring(L"Web Server"), mappings[0].Description);
}

// ===========================================
// Common Port Mapping Scenarios
// ===========================================

TEST(PortPageViewModelTests, CommonScenario_GameServerPortForward)
{
    TestPortMappingInfo gameServer{ L"Minecraft Server", 25565, 25565, L"TCP", L"192.168.1.50" };
    TestPortMappingInfo webServer{ L"Web Server", 80, 80, L"TCP", L"192.168.1.50" };

    EXPECT_FALSE(PortMappingLogic::MappingsMatch(gameServer, webServer));
}

TEST(PortPageViewModelTests, CommonScenario_SamePortDifferentProtocol)
{
    TestPortMappingInfo dnsTcp{ L"DNS", 53, 53, L"TCP", L"192.168.1.1" };
    TestPortMappingInfo dnsUdp{ L"DNS", 53, 53, L"UDP", L"192.168.1.1" };

    EXPECT_FALSE(PortMappingLogic::MappingsMatch(dnsTcp, dnsUdp));
}

TEST(PortPageViewModelTests, CommonScenario_PortRemapping)
{
    TestPortMappingInfo remapped{ L"Web Server", 8080, 80, L"TCP", L"192.168.1.100" };
    TestPortMappingInfo standard{ L"Web Server", 80, 80, L"TCP", L"192.168.1.100" };

    EXPECT_FALSE(PortMappingLogic::MappingsMatch(remapped, standard));
}
