#include "gtest/gtest.h"
#include <string>
#include "../ChatRooms/Room.h"
#include "../ChatRooms/User.h"

TEST(StringSplitting, CMD) {
	std::string msg = "/Hello World";
	size_t firstSpace = msg.find(' ');
	std::string cmd = msg.substr(1, firstSpace - 1);
	std::string param = msg.substr(firstSpace + 1, msg.find(' ', firstSpace + 1));

	ASSERT_EQ(cmd, "Hello");
	ASSERT_EQ(param, "World");

}




int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}