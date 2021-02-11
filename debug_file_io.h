static char *read_entire_file_text(char *path, u64 *size_bytes)
{
    FILE *file = fopen(path, "rb");
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *string = (char *)malloc(file_size + 1);
    fread(string, 1, file_size, file);
    fclose(file);
    string[file_size] = 0;
    if (size_bytes)
    {
        *size_bytes = file_size;
    }
    return string;
}

static u8 *read_entire_file_binary(char *path, u64* size_bytes)
{
    FILE *file = fopen(path, "rb");
    fseek(file, 0, SEEK_END);
    u64 file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    u8 *data = (u8 *)malloc(file_size);
    fread(data, 1, file_size, file);
    fclose(file);
    if (size_bytes)
    {
        *size_bytes = file_size;
    }
    return data;
}