#if !defined(LEN_STRING_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Bryn Murrell $
   ======================================================================== */
struct len_string
{
    u32 buffer_len;
    u32 string_len;
    char *str;
};

inline b32 operator==(const len_string &a, const len_string &b)
{
    if (a.string_len == b.string_len)
    {
        if (streq(a.str, b.str, a.string_len))
        {
            return true;
        }
    }
    return false;
}

// TODO optimize
inline b32 operator!=(const len_string &a, const len_string &b)
{
    return !(a == b);
}

inline b32 operator==(const len_string &a, char *b)
{
    if (streq(a.str, b, a.string_len))
    {
        u32 b_len = strlen(b);
        if (a.string_len == b_len)
        {
            return true;
        }
    }
    return false;
}

// TODO optimize
inline b32 operator!=(const len_string &a, char *b)
{
    return !(a == b);
}

flocal inline void free_l_string(len_string *str)
{
    str->buffer_len = 0;
    str->string_len = 0;
    free(str->str);
}

flocal inline len_string l_string(u32 buffer_len)
{
    len_string str = {};
    str.buffer_len = buffer_len;
    str.string_len = 0;
    str.str = (char *)malloc(str.buffer_len);
    str.str[0] = 0;
    return str;
}

flocal inline void reallocate_len_string(len_string *l, u32 new_size)
{
    l->str = (char *)realloc(l->str, new_size);
    l->buffer_len = new_size;
}

flocal inline void append_to_len_string(len_string *l, char *str, u32 len)
{
    u32 in_str_len = len;
    if (in_str_len + l->string_len > l->buffer_len)
    {
        reallocate_len_string(l, in_str_len + l->string_len + 1);
    }
    for (int i = 0; i < in_str_len; i++)
    {
        l->str[l->string_len + i] = str[i];
    }
    l->string_len += in_str_len;
    l->str[l->string_len] = 0;
}

flocal inline void append_to_len_string(len_string *l, const char *str)
{
    u32 in_str_len = strlen(str);
    if (in_str_len + l->string_len + 1 > l->buffer_len)
    {
        reallocate_len_string(l, in_str_len + l->string_len + 2);
    }
    for (int i = 0; i < in_str_len; i++)
    {
        l->str[l->string_len + i] = str[i];
    }
    l->string_len += in_str_len;
    l->str[l->string_len] = 0;
}

flocal inline len_string l_string(const char *str)
{
    u32 buffer_len = strlen(str) + 1;
    len_string ret = l_string(buffer_len);
    append_to_len_string(&ret, str);
    return ret;
}

// flocal inline len_string l_string(std::string str)
// {
//     u32 buffer_len = str.length() + 2;
//     len_string ret = l_string(buffer_len);

//     LOOP(i, str.length() + 2)
//     {
//         if (i == str.length() + 1)
//         {
//             ret.str[i] = 0;
//         }
//         else
//         {
//             ret.str[i] = str[i];
//         }
//     }

//     return ret;
// }

flocal inline len_string l_string(len_string s)
{
    len_string ret = l_string(s.buffer_len);
    append_to_len_string(&ret, s.str);
    return ret;
}

flocal inline len_string l_string(char *str, u32 len)
{
    u32 buffer_len = len + 1;
    len_string ret = l_string(buffer_len);
    append_to_len_string(&ret, str, len);
    return ret;
}

flocal inline void sub_str_to_null_terminated(char *s, u32 length, char **out)
{

    for (int i = 0; i < length + 1; i++)
    {
        if (i == length)
        {
            (*out)[i] = '\0';
        }
        else
        {
            (*out)[i] = s[i];
        }
    }
}

flocal inline u32 find_first_occurence_of_char(const len_string &str, char c)
{
    LOOP(i, str.string_len)
    {
        if (str.str[i] == c)
        {
            return i;
        }
    }
    return -1;
}

flocal inline void strip_extension(len_string *str)
{

    u32 ext_index = find_first_occurence_of_char(*str, '.');
    str->str[ext_index] = 0;
    str->string_len = ext_index;
}

flocal inline b32 cmp_len_string_ext(const len_string &str, char *ext, u32 ext_len)
{
    ASSERT(ext[0] != '.', "Extensions passed to cmp_len_string_ext() must not start w/ an \'.\'");
    u32 begin = find_first_occurence_of_char(str, '.');
    if (begin != -1)
    {
        int j = 0;
        for (int i = begin + 1; i < str.string_len && j < ext_len; i++)
        {
            if (ext[j] != str.str[i])
            {
                return false;
            }
            j++;
        }
    }
    else
    {
        return false;
    }
    return true;
}

#define LEN_STRING_H
#endif
