#include "gtest/gtest.h"
#include <string>
#include "../ChatRooms/Room.h"
#include "../ChatRooms/User.h"

TEST(StringSplitting, cmdAndParam) {
	std::string msg = "/CREATE_ROOM Test";
	size_t firstSpace = msg.find(' ');
	std::string cmd = msg.substr(1, firstSpace - 1);
	std::string param = msg.substr(firstSpace + 1, msg.find(' ', firstSpace + 1));

	ASSERT_EQ(cmd, "CREATE_ROOM");
	ASSERT_EQ(param, "Test");

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

	EXPECT_TRUE(testRoom1.getSize(), 1);
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