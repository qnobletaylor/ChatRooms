#include "gtest/gtest.h"
#include <string>
#include <regex>
#include <sstream>
#include <iomanip>
#include "../ChatRooms/Room.h"
#include "../ChatRooms/User.h"

TEST(Regex, ipAndPort) {
	std::regex regex{ R"~(^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?):(?:6553[0-5]|655[0-2][0-9]|65[0-4][0-9]{2}|6[0-4][0-9]{3}|[1-5][0-9]{4}|[1-9][0-9]{0,3}|0)$)~" };

	std::string a{ "127.0.0.1:54000" }, b{ "125.12.3.5:65535" }, c{ "1a.23.1.0:1" }, d{ "1.2.3.-:12" };

	EXPECT_TRUE(std::regex_match(a, regex));
	EXPECT_TRUE(std::regex_match(b, regex));
	EXPECT_FALSE(std::regex_match(c, regex));
	EXPECT_FALSE(std::regex_match(d, regex));
}

TEST(Regex, port) {
	std::regex regex{ R"~(^(0|[1-9]\d{0,3}|[1-5]\d{4}|6[0-4]\d{3}|65[0-4]\d{2}|655[0-2]\d|6553[0-5])$)~" };

	std::string a{ "65535" }, b{ "2234" }, c{ "1abc" }, d{ "65536" };

	EXPECT_TRUE(std::regex_match(a, regex));
	EXPECT_TRUE(std::regex_match(b, regex)); //This should be passing...
	EXPECT_FALSE(std::regex_match(c, regex));
	EXPECT_FALSE(std::regex_match(d, regex));
}

TEST(StringSplitting, cmdAndParam) {
	std::string msg1 = "/CREATE_ROOM Test";
	size_t firstSpace = msg1.find(' ');
	std::string cmd1 = msg1.substr(1, firstSpace - 1);
	std::string param1 = msg1.substr(firstSpace + 1, msg1.find(' ', firstSpace + 1));

	ASSERT_EQ(cmd1, "CREATE_ROOM");
	ASSERT_EQ(param1, "Test");

	std::string msg2 = "/LIST_USERS";
	firstSpace = msg2.find(' ');
	std::string cmd2 = msg2.substr(1, firstSpace - 1);
	std::string param2 = msg2.substr(firstSpace + 1, msg2.find(' ', firstSpace + 1));

	ASSERT_EQ(param2, msg2);
}

TEST(UserTest, comparisons) {
	User testUser1{}, testUser2{};
	testUser1.username = testUser2.username = "bob";

	EXPECT_EQ(testUser1, testUser2);

	testUser1.username = "aob";

	EXPECT_LT(testUser1, testUser2);
}

TEST(RoomTest, constructors) {
	Room testRoom1("Room1");
	User testUser{};
	Room testRoom2("Room2", testUser);

	EXPECT_EQ(testRoom1.getName(), "Room1");
	EXPECT_EQ(testRoom2.getName(), "Room2");
	EXPECT_EQ(testRoom2.getCreator(), testUser);
}

TEST(RoomTest, addUser) {
	Room testRoom1("Room1");
	User testUser{};

	EXPECT_TRUE(testRoom1.addUser(testUser));
	EXPECT_FALSE(testRoom1.addUser(testUser));
}

TEST(RoomTest, removeUser) {
	Room testRoom1("Room1");
	User testUser{};
	testRoom1.addUser(testUser);

	EXPECT_TRUE(testRoom1.removeUser(testUser));
	EXPECT_FALSE(testRoom1.removeUser(testUser));
}

TEST(RoomTest, hasUser) {
	Room testRoom1("Room1");
	User testUser1{}, testUser2{};
	testUser2.username = "test";
	testRoom1.addUser(testUser1);

	EXPECT_TRUE(testRoom1.hasUser(testUser1));
	EXPECT_FALSE(testRoom1.hasUser(testUser2));
}

TEST(RoomTest, getUsers) {
	Room testRoom1("Room1");
	User testUser1{}, testUser2{};
	testRoom1.addUser(testUser1);
	testRoom1.addUser(testUser2);
	std::set<User> users = testRoom1.getUsers();

	EXPECT_EQ(*users.find(testUser1), testUser1);
	EXPECT_EQ(*users.find(testUser2), testUser2);
}

TEST(RoomTest, moveUser) {
	Room testRoom1("Room1");
	User testUser{};
	Room testRoom2("Room2");

	ASSERT_TRUE(testRoom1.addUser(testUser));
	EXPECT_TRUE(Room::moveUser(testUser, testRoom1, testRoom2));
	EXPECT_EQ(testUser.currentRoom, "Room2");
}

TEST(RoomTest, getCreator) {
	User testUser{}, testUser2{};
	testUser2.username = "test";
	Room testRoom1("Room1", testUser);
	testRoom1.addUser(testUser2);

	EXPECT_EQ(testRoom1.getCreator(), testUser);
	EXPECT_NE(testRoom1.getCreator(), testUser2);
}

TEST(RoomTest, getSize) {
	User testUser{};
	Room testRoom1("Room1", testUser);

	EXPECT_EQ(testRoom1.getSize(), 1);
}

TEST(RoomTest, changeName) {
	User testUser{};
	Room testRoom1("Room1", testUser);

	EXPECT_TRUE(testRoom1.changeName(testUser, "Room2"));
	EXPECT_EQ(testRoom1.getName(), "Room2");
}





int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}