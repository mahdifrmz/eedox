#include <util.h>

void memcpy(void *dest, const void *src, uint32_t size)
{
    if (!size)
    {
        return;
    }
    if (dest < src)
    {
        for (int64_t i = 0; i < size; i++)
        {
            ((char *)dest)[i] = ((char *)src)[i];
        }
    }
    else if (dest > src)
    {
        for (int64_t i = size - 1; i >= 0; i--)
        {
            ((char *)dest)[i] = ((char *)src)[i];
        }
    }
}

void memset(void *dest, uint8_t val, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++)
    {
        ((char *)dest)[i] = val;
    }
}

void memshift(void *dest, int32_t offset, uint32_t size)
{
    if (!size)
    {
        return;
    }
    if (offset > 0)
    {
        for (int64_t i = size - 1; i >= 0; i--)
        {
            ((char *)dest + offset)[i] = ((char *)dest)[i];
        }
    }
    else if (offset < 0)
    {
        for (int64_t i = 0; i < size; i++)
        {
            ((char *)dest + offset)[i] = ((char *)dest)[i];
        }
    }
}

void strrev(void *ptr, uint32_t len)
{
    for (uint32_t i = 0; i < len / 2; i++)
    {
        char temp = ((char *)ptr)[i];
        ((char *)ptr)[i] = ((char *)ptr)[len - i - 1];
        ((char *)ptr)[len - i - 1] = temp;
    }
}

uint32_t strlen(const char *str)
{
    uint32_t len = 0;
    while (*str != 0)
    {
        len++;
        str++;
    }
    return len;
}

void strcpy(char *dest, const char *src)
{
    for (uint32_t i = 0; src[i] != 0; i++)
    {
        dest[i] = src[i];
    }
}

int8_t strcmp(const char *str1, const char *str2)
{
    uint32_t i;
    for (i = 0; str1[i] != 0 && str2[i] != 0; i++)
    {
        if (str1[i] > str2[i])
        {
            return 1;
        }
        else if (str1[i] < str2[i])
        {
            return -1;
        }
    }

    if (str1[i] != 0)
    {
        return 1;
    }
    else if (str2[i] != 0)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

void tostr_u(char *buffer, uint32_t i)
{
    uint32_t idx = 0;
    if (!i)
    {
        buffer[idx++] = '0';
        buffer[idx] = 0;
    }
    else
    {
        while (i > 0)
        {
            buffer[idx++] = '0' + i % 10;
            i /= 10;
        }
        buffer[idx] = 0;
    }
    strrev(buffer, idx);
}

char dec_to_hex(uint8_t d)
{
    if (d < 10)
    {
        return '0' + d;
    }
    else
    {
        return 'a' + d - 10;
    }
}

void tostr_x(char *buffer, uint32_t h)
{
    buffer[0] = '0';
    buffer[1] = 'x';
    buffer[10] = '\0';
    for (uint32_t i = 0; i < 8; i++)
    {
        buffer[9 - i] = dec_to_hex(h % 16);
        h /= 16;
    }
}

int32_t _bin_search(uint32_t *ptr, int32_t from, int32_t to, uint32_t value)
{
    if (to < from)
    {
        return -1;
    }
    uint32_t mid = (from + to) / 2;
    if (ptr[mid] == value)
    {
        return mid;
    }
    else if (ptr[mid] > value)
    {
        return _bin_search(ptr, from, mid - 1, value);
    }
    else
    {
        return _bin_search(ptr, mid + 1, to, value);
    }
}

int32_t bin_search(uint32_t *ptr, int32_t len, int32_t value)
{
    if (len == 0)
    {
        return -1;
    }
    return _bin_search(ptr, 0, len - 1, value);
}

int32_t partial_print(char *buffer, int32_t index, const char *message, va_list args)
{
    char *beg = buffer;
    int32_t i;
    for (i = index; (uint32_t)i < strlen(message) && buffer - beg < PARTIAL_PRINT_BUFFER_SIZE; i++)
    {
        char c = message[i];
        if (c == '%')
        {
            char ty = message[++i];
            if (ty == '%')
            {
                *(buffer++) = '%';
            }
            else if (ty == 'u')
            {
                uint32_t num = va_arg(args, uint32_t);
                char numbuf[11];
                tostr_u(numbuf, num);
                strcpy(buffer, numbuf);
                buffer += strlen(numbuf);
            }
            else if (ty == 'x')
            {
                uint32_t num = va_arg(args, uint32_t);
                char numbuf[11];
                tostr_x(numbuf, num);
                strcpy(buffer, numbuf);
                buffer += strlen(numbuf);
            }
            else if (ty == 's')
            {
                char *str = va_arg(args, char *);
                if (str)
                {
                    strcpy(buffer, str);
                    buffer += strlen(str);
                }
            }
            else
            {
                return -1;
            }
        }
        else
        {
            *(buffer++) = c;
        }
    }
    return i;
}

int32_t sprintf(char *buffer, const char *message, ...)
{
    va_list args;
    char *beg = buffer;
    va_start(args, message);
    char local[PARTIAL_PRINT_BUFFER_SIZE + 16];
    int32_t index = 0;
    while ((uint32_t)index < strlen(message))
    {
        memset(local, 0, sizeof(local));
        index = partial_print(local, index, message, args);
        if (index == -1)
        {
            return -1;
        }
        strcpy(buffer, local);
        buffer += strlen(local);
    }
    va_end(args);
    return buffer - beg;
}