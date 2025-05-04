#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "kevs.h"

static int read_file(const char *path, char **out, size_t *out_len) {
  FILE *file = fopen(path, "r");
  if (file == NULL) {
    printf("error: failed to open file\n");
    return 1;
  }

  fseek(file, 0, SEEK_END);
  size_t len = ftell(file);
  rewind(file);

  char *ptr = (char *)malloc(len + 1);

  fread(ptr, 1, len, file);
  ptr[len] = '\0';

  fclose(file);

  *out = ptr;
  *out_len = len;

  return 0;
}

int main() {
  const char *file = "example.kevs";

  char *data = NULL;
  size_t data_len = 0;
  if (read_file(file, &data, &data_len) != 0) {
    return 1;
  }

  KevsTable root = {};

  char err_buf[8193] = {};
  const KevsParams params = {
      .file = kevs_str_from_cstr(file),
      .content = {.ptr = data, .len = data_len},
      .err_buf = err_buf,
      .err_buf_len = sizeof(err_buf) - 1,
  };

  KevsError err = kevs_parse(&root, params);
  if (err != NULL) {
    printf("error: failed to parse: %s\n", err);
    return 1;
  }

  KevsTable dependencies = {};
  kevs_table_table(root, "dependencies", &dependencies);

  for (size_t i = 0; i < dependencies.len; i++) {
    char *name = kevs_str_dup(dependencies.ptr[i].key);
    KevsTable dependency = {};
    kevs_table_table(dependencies, name, &dependency);
    printf("%s:\n", name);
    if (kevs_table_has(dependency, "url")) {
      char *v = NULL;
      kevs_table_string(dependency, "url", &v);
      printf("  url = %s\n", v);
    }
    if (kevs_table_has(dependency, "hash")) {
      char *v = NULL;
      kevs_table_string(dependency, "hash", &v);
      printf("  hash = %s\n", v);
    }
    if (kevs_table_has(dependency, "path")) {
      char *v = NULL;
      kevs_table_string(dependency, "path", &v);
      printf("  path = %s\n", v);
    }
  }

  return 0;
}
