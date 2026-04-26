#define _POSIX_C_SOURCE 200809L

#include "rbuflogd/producer.h"
#include <stdio.h>
#include <string.h>

#define TEST_MSG_COUNT 10
#define TEST_MSG_MAX_LEN 256

static const char * producer_default_name = "prod1";
static const char * producer_default_category = "cat1";

static void print_usage(const char * prog_name);

static int parse_cli_args(
  int argc,
  char ** argv,
  const char ** out_producer_name,
  const char ** out_producer_category,
  const char ** out_message,
  rbuflogd_log_level_t * out_log_level);

static int parse_log_level_arg(const char * value, rbuflogd_log_level_t * out_level);



int main(int argc, char ** argv) {
  const char * producer_name = NULL;
  const char * producer_category = NULL;
  const char * test_message = NULL;
  rbuflogd_log_level_t log_level = RBUF_LOG_LEVEL_INFO;
  const int parse_result = parse_cli_args(
    argc, argv, 
    &producer_name, 
    &producer_category,
    &test_message,
    &log_level);

  if (parse_result == 1) {
    return 0;
  }

  if (parse_result != 0) {
    return -1;
  }

  printf("Starting producer test...\n");
  printf("Using producer=\"%s\", category=\"%s\"\n", producer_name, producer_category);

  rbuflogd_producer_t producer = {
    .state = NULL,
  };

  if (rbuflogd_producer_open(
    &producer, 
    producer_name) == -1) {
    perror("rbuflogd_producer_open");
    return -1;
  }

  int i = 0; 
  char log_msg[TEST_MSG_MAX_LEN];
  if (test_message != NULL) {
    snprintf(log_msg, sizeof(log_msg), "%s", test_message);
  }
  else {
    snprintf(log_msg, sizeof(log_msg), "test message %d", i);
  }

  const int res = rbuflogd_producer_log(
    &producer, 
    log_level,
    producer_category, 
    log_msg);

  if (res == -1) {
    printf("Buffer full\n");
  }
  else {
    printf("Wrote: \"%s\"\n", log_msg);
  }


  rbuflogd_producer_close(&producer);
  return 0;
}


static void print_usage(const char * prog_name) {
  printf("Usage: %s [-p|--producer NAME] [-c|--category CATEGORY] [-m|--message TEXT] [-l|--log-level LEVEL]\n", prog_name);
  printf("  LEVEL can be: debug, info, warning, error\n");
}

static int parse_cli_args(
  int argc,
  char ** argv,
  const char ** out_producer_name,
  const char ** out_producer_category,
  const char ** out_message,
  rbuflogd_log_level_t * out_log_level) {
  const char * producer_name = producer_default_name;
  const char * producer_category = producer_default_category;
  const char * test_message = NULL;
  rbuflogd_log_level_t log_level = RBUF_LOG_LEVEL_INFO;

  if (out_producer_name == NULL || out_producer_category == NULL ||
      out_message == NULL || out_log_level == NULL) {
    return -1;
  }

  for (int i = 1; i < argc; i++) {
    if ((strcmp(argv[i], "-p") == 0) || (strcmp(argv[i], "--producer") == 0)) {
      if ((i + 1) >= argc) {
        fprintf(stderr, "Missing value for %s\n", argv[i]);
        print_usage(argv[0]);
        return -1;
      }
      producer_name = argv[++i];
      continue;
    }

    if ((strcmp(argv[i], "-c") == 0) || (strcmp(argv[i], "--category") == 0)) {
      if ((i + 1) >= argc) {
        fprintf(stderr, "Missing value for %s\n", argv[i]);
        print_usage(argv[0]);
        return -1;
      }
      producer_category = argv[++i];
      continue;
    }

    if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)) {
      print_usage(argv[0]);
      return 1;
    }

    if ((strcmp(argv[i], "-l") == 0) || (strcmp(argv[i], "--log-level") == 0)) {
      if ((i + 1) >= argc) {
        fprintf(stderr, "Missing value for %s\n", argv[i]);
        print_usage(argv[0]);
        return -1;
      }

      if (parse_log_level_arg(argv[++i], &log_level) != 0) {
        fprintf(stderr, "Invalid log level: %s\n", argv[i]);
        print_usage(argv[0]);
        return -1;
      }
      continue;
    }

    if ((strcmp(argv[i], "-m") == 0) || (strcmp(argv[i], "--message") == 0)) {
      if ((i + 1) >= argc) {
        fprintf(stderr, "Missing value for %s\n", argv[i]);
        print_usage(argv[0]);
        return -1;
      }
      test_message = argv[++i];
      continue;
    }

    fprintf(stderr, "Unknown option: %s\n", argv[i]);
    print_usage(argv[0]);
    return -1;
  }

  *out_producer_name = producer_name;
  *out_producer_category = producer_category;
  *out_message = test_message;
  *out_log_level = log_level;
  return 0;
}

static int parse_log_level_arg(const char * value, rbuflogd_log_level_t * out_level) {
  if (value == NULL || out_level == NULL) {
    return -1;
  }

  if (strcmp(value, "debug") == 0) {
    *out_level = RBUF_LOG_LEVEL_DEBUG;
    return 0;
  }

  if (strcmp(value, "info") == 0) {
    *out_level = RBUF_LOG_LEVEL_INFO;
    return 0;
  }

  if (strcmp(value, "warning") == 0) {
    *out_level = RBUF_LOG_LEVEL_WARNING;
    return 0;
  }

  if (strcmp(value, "error") == 0) {
    *out_level = RBUF_LOG_LEVEL_ERROR;
    return 0;
  }

  return -1;
}