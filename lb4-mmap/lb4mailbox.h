#pragma once

#include <cstdint>
#include <string>

uint32_t crc32(const char* buf, uint32_t size);

class MailboxEntry {
public:
    MailboxEntry(const char* content, uint32_t size);
    MailboxEntry(const std::string& content);

    std::string getContent();

    void write(char* data);
private:
    uint32_t content_size;
    uint32_t checksum;
    char* content;
};

class MailBox {
public:
    MailBox(const std::string& name);
    MailBox(const std::string& name, uint32_t max_size);
    uint32_t getMaxSize();
    uint64_t getCurrentSize();
    uint32_t getEntriesCount();

    void addEntry(MailboxEntry* entry);
    MailboxEntry* readEntry(uint32_t index, bool del = false);
    void deleteEntry(uint32_t index);
    void deleteAllEntries();

    static uint32_t getMailboxCount();

private:
    std::string filename;
    uint32_t max_size = 0;
    uint32_t current_index = 0;

    static constexpr uint32_t HEADER_SIZE = 8;

    uint32_t getMessageAbsoluteAddress(char* data, uint32_t index);
    uint32_t getMessageSize(char* data, uint32_t index);
    uint32_t getContentStart();
};
