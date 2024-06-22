CC = gcc
# -Wall for all warnings
# -Wextra for even more warnings
CFLAGS = -Wall -Wextra -Iinclude
LDFLAGS = -lnsl -lpthread
SRCDIR = src
OBJDIR = build
BINDIR = bin
INCLUDEDIR = include

SRCFILES_COMMON = $(wildcard $(SRCDIR)/*.c)
OBJFILES_COMMON = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCFILES_COMMON))

SRCDIR_JOB_COMMANDER = $(SRCDIR)/jobCommander
OBJDIR_JOB_COMMANDER = $(OBJDIR)/jobCommander
SRCFILES_JOB_COMMANDER = $(wildcard $(SRCDIR_JOB_COMMANDER)/*.c)
OBJFILES_JOB_COMMANDER = $(patsubst $(SRCDIR_JOB_COMMANDER)/%.c,$(OBJDIR_JOB_COMMANDER)/%.o,$(SRCFILES_JOB_COMMANDER))

SRCDIR_JOB_EXECUTOR_SERVER = $(SRCDIR)/jobExecutorServer
OBJDIR_JOB_EXECUTOR_SERVER = $(OBJDIR)/jobExecutorServer
SRCFILES_JOB_EXECUTOR_SERVER = $(wildcard $(SRCDIR_JOB_EXECUTOR_SERVER)/*.c)
OBJFILES_JOB_EXECUTOR_SERVER = $(patsubst $(SRCDIR_JOB_EXECUTOR_SERVER)/%.c,$(OBJDIR_JOB_EXECUTOR_SERVER)/%.o,$(SRCFILES_JOB_EXECUTOR_SERVER))

TESTDIR = tests

all: $(BINDIR) $(BINDIR)/jobExecutorServer $(BINDIR)/jobCommander $(BINDIR)/progDelay

$(BINDIR)/jobExecutorServer: $(OBJFILES_JOB_EXECUTOR_SERVER) $(OBJFILES_COMMON) | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $(OBJFILES_JOB_EXECUTOR_SERVER) $(OBJFILES_COMMON) $(LDFLAGS)

$(OBJDIR_JOB_EXECUTOR_SERVER)/%.o: $(SRCDIR_JOB_EXECUTOR_SERVER)/%.c | $(OBJDIR_JOB_EXECUTOR_SERVER)
	$(CC) $(CFLAGS) -c $< -o $@

$(BINDIR)/jobCommander: $(OBJFILES_JOB_COMMANDER) $(OBJFILES_COMMON) | $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $(OBJFILES_JOB_COMMANDER) $(OBJFILES_COMMON) $(LDFLAGS)

$(OBJDIR_JOB_COMMANDER)/%.o: $(SRCDIR_JOB_COMMANDER)/%.c | $(OBJDIR_JOB_COMMANDER)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BINDIR)/progDelay: $(TESTDIR)/progDelay.c
	$(CC) $(CFLAGS) -o $(BINDIR)/progDelay $(TESTDIR)/progDelay.c $(LDFLAGS)

# Creating directories
# ---------------------

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR_JOB_EXECUTOR_SERVER):
	mkdir -p $(OBJDIR_JOB_EXECUTOR_SERVER)

$(OBJDIR_JOB_COMMANDER):
	mkdir -p $(OBJDIR_JOB_COMMANDER)

$(BINDIR):
	mkdir -p $(BINDIR)

# ---- end of creating directories ---------

clean:
	rm -fr $(OBJDIR) $(BINDIR)

.PHONY: all clean
