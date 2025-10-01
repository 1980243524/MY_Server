#include "user.h"

void UserManager::UserOnline(int32_t fd) { users_.emplace(fd, User(fd)); }