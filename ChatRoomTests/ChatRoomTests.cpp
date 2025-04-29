#include "gtest/gtest.h"
import User;

TEST(ChatRoomTests, Constructor) {
	Message msg("Ted", "12:34:56");

	EXPECT_EQ(msg.message, "Ted");
	EXPECT_EQ(msg.timeStamp, "12");
}




int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}