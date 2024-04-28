#include <cstdint>
#include <iostream>

#include "lb4mailbox.h"

void printMbStats(MailBox* mb) {
    printf("Max size: %d\n", mb->getMaxSize());
    printf("Current size (in bytes): %llu\n", mb->getCurrentSize());
    printf("Entries count: %d\n", mb->getEntriesCount());
    printf("-------------------------------\n");
}

void menu_mailbox(MailBox* mb) {
    std::string input;
    int sel;
    while (true) {
        printf("Pick action:\n");
        printf(" 0 - Close mailbox\n");
        printf(" 1 - Add entry\n");
        printf(" 2 - Read entry (without deletion)\n");
        printf(" 3 - Read entry (with deletion)\n");
        printf(" 4 - Delete entry\n");
        printf(" 5 - Delete all entries\n");
        printf(" 6 - Get stats\n");
        printf("Select: ");
        std::getline(std::cin, input);

        try {
            sel = std::stoi(input);
        } catch (std::exception const& ex) {
            printf("Error: %s\n", ex.what());
            continue;
        }
        if(sel < 0 || sel > 6)
            continue;

        switch (sel) {
        case 0:
            return;
        case 1: {
            printf("  Entry text: ");
            std::getline(std::cin, input);
            try {
                mb->addEntry(new MailboxEntry(input));
            } catch (std::exception const& ex) {
                printf("Error: %s\n", ex.what());
                break;
            }
            break;
        }
        case 2:
        case 3:
        case 4: {
            printf("  Entry index (0-%d): ", mb->getEntriesCount()-1);
            std::getline(std::cin, input);

            int idx;
            try {
                idx = std::stoi(input);
            } catch (std::exception const& ex) {
                printf("Error: %s\n", ex.what());
                break;
            }
            if(idx < 0 || idx >= mb->getEntriesCount()) {
                printf("Wrong index!\n");
                break;
            }

            if(sel == 2 || sel == 3) {
                MailboxEntry* e;
                try {
                    e = mb->readEntry(idx, sel == 3);
                } catch (std::exception const& ex) {
                    printf("Error: %s\n", ex.what());
                    break;
                }

                printf("Content: %s\n", e->getContent().c_str());
            } else {
                mb->deleteEntry(idx);
            }

            break;
        }
        case 5: {
            try {
                mb->deleteAllEntries();
            } catch (std::exception const& ex) {
                printf("Error: %s\n", ex.what());
                break;
            }

            break;
        }
        case 6: {
            try {
                printMbStats(mb);
            } catch (std::exception const& ex) {
                printf("Error: %s\n", ex.what());
                break;
            }

            break;
        }
        }
    }
}

void menu_main() {
    std::string input;
    int sel;
    while (true) {
        printf("Pick action:\n");
        printf(" 0 - Exit\n");
        printf(" 1 - Print mailboxes count\n");
        printf(" 2 - Create mailbox\n");
        printf(" 3 - Open mailbox\n");
        printf("Select: ");
        std::getline(std::cin, input);

        try {
            sel = std::stoi(input);
        } catch (std::exception const& ex) {
            printf("Error: %s\n", ex.what());
            continue;
        }
        if(sel < 0 || sel > 3)
            continue;

        switch (sel) {
        case 0:
            return;
        case 1: {
            printf("Count: %d\n", MailBox::getMailboxCount());
            break;
        }
        case 2: {
            printf("  Mailbox name: ");
            std::getline(std::cin, input);
            std::string name = input;
            printf("  Mailbox max size: ");
            std::getline(std::cin, input);
            int size;
            try {
                size = std::stoi(input);
            } catch (std::exception const& ex) {
                printf("Error: %s\n", ex.what());
                break;
            }
            if(size <= 0) {
                printf("Size must be bigger than 0\n");
                break;
            }

            menu_mailbox(new MailBox(name+".mb", size));
            break;
        }
        case 3: {
            printf("  Mailbox name: ");
            std::getline(std::cin, input);
            menu_mailbox(new MailBox(input+".mb"));
            break;
        }
        }
    }
}

int main() {
    menu_main();

    return 0;
}
