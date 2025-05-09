#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERS 10
#define MAX_RESOURCES 10
#define MAX_NAME_LEN 20
#define MAX_ACL_ENTRIES 10
#define MAX_CAPABILITIES 10

typedef enum {
    READ = 1,
    WRITE = 2,
    EXECUTE = 4
} Permission;

typedef struct {
    char name[MAX_NAME_LEN];
} User;

typedef struct {
    char name[MAX_NAME_LEN];
} Resource;

// ACL Entry
typedef struct {
    char userName[MAX_NAME_LEN];
    int permissions; 
} ACLEntry;

typedef struct {
    Resource resource;
    ACLEntry entries[MAX_ACL_ENTRIES];
    int aclCount;
} ACLControlledResource;

// Capability Entry
typedef struct {
    char resourceName[MAX_NAME_LEN];
    int permissions;
} Capability;

typedef struct {
    User user;
    Capability capabilities[MAX_CAPABILITIES];
    int capCount;
} CapabilityUser;

void printPermissions(int perm) {
    if (perm & READ) printf("Read ");
    if (perm & WRITE) printf("Write ");
    if (perm & EXECUTE) printf("Execute ");
}

int hasPermission(int userPerm, int requiredPerm) {
    return (userPerm & requiredPerm) == requiredPerm;
}

// ACL Check
void checkACLAccess(ACLControlledResource *res, const char *userName, int perm) {
    for (int i = 0; i < res->aclCount; i++) {
        if (strcmp(res->entries[i].userName, userName) == 0) {
            printf("ACL Check: User %s requests ", userName);
            printPermissions(perm);
            printf("on %s: ", res->resource.name);
            if (hasPermission(res->entries[i].permissions, perm))
                printf("Access GRANTED\n");
            else
                printf("Access DENIED\n");
            return;
        }
    }

    printf("ACL Check: User %s has NO entry for resource %s: Access DENIED\n", userName, res->resource.name);
}

// Capability Check
void checkCapabilityAccess(CapabilityUser *user, const char *resourceName, int perm) {
    for (int i = 0; i < user->capCount; i++) {
        if (strcmp(user->capabilities[i].resourceName, resourceName) == 0) {
            printf("Capability Check: User %s requests ", user->user.name);
            printPermissions(perm);
            printf("on %s: ", resourceName);
            if (hasPermission(user->capabilities[i].permissions, perm))
                printf("Access GRANTED\n");
            else
                printf("Access DENIED\n");
            return;
        }
    }
    printf("Capability Check: User %s has NO capability for resource %s: Access DENIED\n", user->user.name, resourceName);
}

int main() {
    // Users and Resources
    User users[MAX_USERS] = {{"Alice"}, {"Bob"}, {"Charlie"}, {"Azmain"}, {"Tarunno"},{"Faysal"},{"Bayern Munich"}};
    Resource resources[MAX_RESOURCES] = {{"File1"}, {"File2"}, {"File3"}, {"File4"}, {"File5"},{"File6"}};

    // ACL Setup
    ACLControlledResource aclResources[MAX_RESOURCES] = {
        {resources[0], {{"Alice", READ | WRITE}, {"Bob", READ}}, 2},
        {resources[1], {{"Charlie", READ | EXECUTE}}, 1},
        {resources[2], {{"Alice", READ}, {"Charlie", WRITE}}, 2},
        {resources[3], {{"Azmain", READ | EXECUTE}}, 1},
        {resources[4], {{"Tarunno", WRITE}}, 1},
        {resources[5], {{"Bayern Munich", READ | EXECUTE}}, 1}
    };

    // Capability Setup
    CapabilityUser capUsers[MAX_USERS] = {
        {users[0], {{"File1", READ | WRITE}, {"File3", READ}}, 2},
        {users[1], {{"File1", READ}}, 1},
        {users[2], {{"File2", EXECUTE}, {"File3", WRITE}}, 2},
        {users[3], {{"File4", READ | EXECUTE}}, 1},
        {users[4], {{"File5", WRITE}}, 1},
        {users[5], {{"File5", WRITE}}, 1},
        {users[6], {{"File6", READ | EXECUTE}}, 1},
        {users[6], {{"File6", WRITE}}, 1},
        {users[5], {{"File4", READ | EXECUTE}}, 1},
    };

    // ACL
    checkACLAccess(&aclResources[0], "Alice", READ);
    checkACLAccess(&aclResources[0], "Bob", WRITE);
    checkACLAccess(&aclResources[0], "Charlie", READ);
    checkACLAccess(&aclResources[3], "Azmain", EXECUTE);
    checkACLAccess(&aclResources[4], "Faysal", READ);
    checkACLAccess(&aclResources[2], "Tarunno", WRITE);
    checkACLAccess(&aclResources[5], "Bayern Munich", EXECUTE);
    checkACLAccess(&aclResources[1], "Charlie", EXECUTE);
    checkACLAccess(&aclResources[6], "Bayern Munich", WRITE);
    checkACLAccess(&aclResources[2], "Bayern Munich", READ);
    checkACLAccess(&aclResources[3], "Faysal", WRITE);
    checkACLAccess(&aclResources[4], "Azmain", READ);
    checkACLAccess(&aclResources[5], "Tarunno", EXECUTE);

    //  CCA
    checkCapabilityAccess(&capUsers[0], "File1", WRITE);
    checkCapabilityAccess(&capUsers[1], "File1", WRITE);
    checkCapabilityAccess(&capUsers[2], "File2", EXECUTE);
    checkCapabilityAccess(&capUsers[2], "File3", WRITE);
    checkCapabilityAccess(&capUsers[3], "File4", EXECUTE);
    checkCapabilityAccess(&capUsers[4], "File5", READ);
    checkCapabilityAccess(&capUsers[5], "File5", WRITE);
    checkCapabilityAccess(&capUsers[6], "File6", READ);
    checkCapabilityAccess(&capUsers[6], "File6", WRITE);
    checkCapabilityAccess(&capUsers[5], "File4", EXECUTE);
    checkCapabilityAccess(&capUsers[0], "File3", READ);
    checkCapabilityAccess(&capUsers[1], "File1", READ);

    return 0;
}
