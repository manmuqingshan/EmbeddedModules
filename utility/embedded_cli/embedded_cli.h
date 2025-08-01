#ifndef __EMBEDDED_CLI_H__
#define __EMBEDDED_CLI_H__

#ifdef __cplusplus

extern "C" {
#else
#include <stdbool.h>
#endif
// cstdint is available only since C++11, so use C header
#include <stddef.h>
#include <stdint.h>

#include "modules.h"

// used for proper alignment of cli buffer
#if UINTPTR_MAX == 0xFFFF
#define CLI_UINT uint16_t
#elif UINTPTR_MAX == 0xFFFFFFFF
#define CLI_UINT uint32_t
#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFFu
#define CLI_UINT uint64_t
#else
#error unsupported pointer size
#endif

#define CLI_UINT_SIZE (sizeof(CLI_UINT))
// convert size in bytes to size in terms of CLI_UINTs (rounded up
// if bytes is not divisible by size of single CLI_UINT)
#define BYTES_TO_CLI_UINTS(bytes) \
    (((bytes) + CLI_UINT_SIZE - 1) / CLI_UINT_SIZE)

typedef struct CliCommand CliCommand;
typedef struct CliCommandBinding CliCommandBinding;
typedef struct EmbeddedCli EmbeddedCli;
typedef struct EmbeddedCliConfig EmbeddedCliConfig;

struct CliCommand {
    /**
   * Name of the command.
   * In command "set led 1 1" "set" is name
   */
    const char* name;

    /**
   * String of arguments of the command.
   * In command "set led 1 1" "led 1 1" is string of arguments
   * Is ended with double 0x00 char
   * Use tokenize functions to easily get individual tokens
   */
    char* args;
};

/**
 * Struct to describe binding of command to function and
 */
struct CliCommandBinding {
    /**
   * Binding function for when command is received.
   * If null, default callback (onCommand) will be called.
   * @param cli - pointer to cli that is calling this binding
   * @param args - string of args (if tokenizeArgs is false) or tokens otherwise
   * @param context
   */
    void (*func)(EmbeddedCli* cli, char* args, void* context);

    /**
   * Name of command to bind. Should not be NULL.
   */
    const char* name;

    /**
   * Help string for usage like "<cmd> <arg1> <arg2> [<arg3>]".
   * Can be NULL if no help is provided.
   */
    const char* usage;

    /**
   * Help string that will be displayed when "help <cmd>" is executed.
   * Can have multiple lines separated with "\r\n"
   * Can be NULL if no help is provided.
   */
    const char* help;

    /**
   * Flag to perform tokenization before calling binding function.
   */
    bool autoTokenizeArgs;

    /**
   * Pointer to any specific app context that is required for this binding.
   * It will be provided in binding callback.
   */
    void* context;
};

struct EmbeddedCli {
    /**
   * Should write char to connection (fallback if writeString is not set)
   * @param cli - pointer to cli that executed this function
   * @param c   - actual character to write
   */
    void (*writeChar)(EmbeddedCli* cli, char c);

    /**
   * Called when command is received and command not found in list of
   * command bindings (or binding function is null).
   * @param cli     - pointer to cli that executed this function
   * @param command - pointer to received command
   */
    void (*onCommand)(EmbeddedCli* cli, CliCommand* command);

    /**
   * Pointer to actual implementation, do not use.
   */
    void* _impl;
};

/**
 * Configuration to create CLI
 */
struct EmbeddedCliConfig {
    /**
   * Invitation string. Is printed at the beginning of each line with user
   * input
   */
    const char* invitation;

    /**
   * Size of buffer that is used to store characters until they're processed
   */
    uint16_t rxBufferSize;

    /**
   * Size of buffer that is used to store current input that is not yet
   * sended as command (return not pressed yet)
   */
    uint16_t cmdBufferSize;

    /**
   * Size of buffer that is used to store previously entered commands
   * Only unique commands are stored in buffer. If buffer is smaller than
   * entered command (including arguments), command is discarded from history
   */
    uint16_t historyBufferSize;

    /**
   * Maximum amount of bindings that can be added via addBinding function.
   * Cli increases takes extra bindings for internal commands:
   * - help
   */
    uint16_t maxBindingCount;

    /**
   * Buffer to use for cli and all internal structures. If NULL, memory will
   * be allocated dynamically. Otherwise this buffer is used and no
   * allocations are made
   */
    CLI_UINT* cliBuffer;

    /**
   * Size of buffer for cli and internal structures (in bytes).
   */
    uint16_t cliBufferSize;

    /**
   * Whether autocompletion should be enabled.
   * If false, autocompletion is disabled but you still can use 'tab' to
   * complete current command manually.
   */
    bool enableAutoComplete;

    /**
   * Whether color output should be enabled.
   * If false, color output is disabled and all color codes are ignored.
   */
    bool enableColorOutput;
};

/**
 * Returns pointer to default configuration for cli creation. It is safe to
 * modify it and then send to embeddedCliNew().
 * Returned structure is always the same so do not free and try to use it
 * immediately.
 * Default values:
 * <ul>
 * <li>rxBufferSize = 64</li>
 * <li>cmdBufferSize = 64</li>
 * <li>historyBufferSize = 128</li>
 * <li>cliBuffer = NULL (use dynamic allocation)</li>
 * <li>cliBufferSize = 0</li>
 * <li>maxBindingCount = 8</li>
 * <li>enableAutoComplete = true</li>
 * </ul>
 * @return configuration for cli creation
 */
EmbeddedCliConfig* embeddedCliDefaultConfig(void);

/**
 * Returns how many space in config buffer is required for cli creation
 * If you provide buffer with less space, embeddedCliNew will return NULL
 * This amount will always be divisible by CLI_UINT_SIZE so allocated buffer
 * and internal structures can be properly aligned
 * @param config
 * @return
 */
uint16_t embeddedCliRequiredSize(EmbeddedCliConfig* config);

/**
 * Create new CLI.
 * Memory is allocated dynamically if cliBuffer in config is NULL.
 * After CLI is created, override function pointers to start using it
 * @param config - config for cli creation
 * @return pointer to created CLI
 */
EmbeddedCli* embeddedCliNew(EmbeddedCliConfig* config);

/**
 * Same as calling embeddedCliNew with default config.
 * @return
 */
EmbeddedCli* embeddedCliNewDefault(void);

/**
 * Receive character and put it to internal buffer
 * Actual processing is done inside embeddedCliProcess
 * You can call this function from something like interrupt service routine,
 * just make sure that you call it only from single place. Otherwise input
 * might get corrupted
 * @param cli
 * @param c   - received char
 */
void embeddedCliReceiveChar(EmbeddedCli* cli, char c);

/**
 * Receive buffer and put it to internal buffer
 * Actual processing is done inside embeddedCliProcess
 * You can call this function from something like interrupt service routine,
 * just make sure that you call it only from single place. Otherwise input
 * might get corrupted
 * @param cli
 * @param buffer
 * @param len
 */
void embeddedCliReceiveBuffer(EmbeddedCli* cli, const char* buffer, size_t len);

/**
 * Process rx/tx buffers. Command callbacks are called from here
 * @param cli
 */
void embeddedCliProcess(EmbeddedCli* cli);

/**
 * Add specified binding to list of bindings. If list is already full, binding
 * is not added and false is returned
 * @param cli
 * @param binding
 * @return true if binding was added, false otherwise
 */
bool embeddedCliAddBinding(EmbeddedCli* cli, CliCommandBinding binding);

/**
 * Delete binding with specified name from list of bindings
 * @param cli
 * @param name
 * @return true if binding was deleted, false otherwise
 */
bool embeddedCliDelBinding(EmbeddedCli* cli, const char* name);

/**
 * Print specified string and account for currently entered but not
 * submitted command. Current command is deleted, provided string is printed
 * (with new line) after that current command is printed again, so user can
 * continue typing it.
 * @param cli
 * @param string
 */
void embeddedCliPrint(EmbeddedCli* cli, const char* string);

/**
 * Free allocated for cli memory
 * @param cli
 */
void embeddedCliFree(EmbeddedCli* cli);

/**
 * @brief Get command entry from cli and switch to it (Professional only)
 * @param cli
 * @return command entry
 */
void (*embeddedCliSwitchToCommandEntry(EmbeddedCli* cli, const char* name))(
    EmbeddedCli* cli, char* args, void* context);

/**
 * Perform tokenization of arguments string. Original string is modified and
 * should not be used directly (only inside other token functions).
 * Individual tokens are separated by single 0x00 char, double 0x00 is put at
 * the end of token list. After calling this function, you can use other
 * token functions to get individual tokens and token count.
 *
 * Important: Call this function only once. Otherwise information will be lost
 * if more than one token existed
 * @param args - string to tokenize (must have extra writable char after 0x00)
 * @return
 */
void embeddedCliTokenizeArgs(char* args);

/**
 * Return specific token from tokenized string
 * @param tokenizedStr
 * @param pos (counted from 1)
 * @return token
 */
const char* embeddedCliGetToken(const char* tokenizedStr, int16_t pos);

/**
 * @brief Pop first token from tokenized string
 * @param tokenizedStr pointer to tokenized string
 * @return token or NULL if no token left
 */
const char* embeddedCliPopToken(char** tokenizedStr);

/**
 * Same as embeddedCliGetToken but works on non-const buffer
 * @param tokenizedStr
 * @param pos (counted from 1)
 * @return token
 */
char* embeddedCliGetTokenVariable(char* tokenizedStr, int16_t pos);

/**
 * Find token in provided tokens string and return its position (counted from 1)
 * If no such token is found - 0 is returned.
 * @param tokenizedStr
 * @param token - token to find
 * @return position (from 1) or 0 if no such token found
 */
uint16_t embeddedCliFindToken(const char* tokenizedStr, const char* token);

/**
 * @brief Find token starting with specified string in provided tokens string
 * @param  tokenizedStr
 * @param  token
 * @retval position (from 1) or 0 if no such token found
 */
uint16_t embeddedCliFindTokenStartswith(const char* tokenizedStr,
                                        const char* token);

/**
 * @brief Find token ending with specified string in provided tokens string
 * @param  tokenizedStr
 * @param  token
 * @retval position (from 1) or 0 if no such token found
 */
uint16_t embeddedCliFindTokenEndswith(const char* tokenizedStr,
                                      const char* token);

/**
 * Check if tokenized string contains specified token at specified position
 * @param tokenizedStr
 * @param token
 * @param pos (counted from 1)
 * @return true if token is at specified position, false otherwise
 */
bool embeddedCliCheckToken(const char* tokenizedStr, const char* token,
                           int16_t pos);

/**
 * @brief Check if tokenized string starts with specified token at specified
 * position
 * @param  tokenizedStr
 * @param  token
 * @param  pos (counted from 1)
 * @retval true if token at specified position starts with specified token
 */
bool embeddedCliCheckTokenStartswith(const char* tokenizedStr,
                                     const char* token, int16_t pos);

/**
 * Check if tokenized string ends with specified token at specified position
 * @param  tokenizedStr
 * @param  token
 * @param  pos (counted from 1)
 * @retval true if token at specified position ends with specified token
 */
bool embeddedCliCheckTokenEndswith(const char* tokenizedStr, const char* token,
                                   int16_t pos);

/**
 * Return number of tokens in tokenized string
 * @param tokenizedStr
 * @return number of tokens
 */
uint16_t embeddedCliGetTokenCount(const char* tokenizedStr);

/**
 * Print help for current running command
 * @param cli
 */
void embeddedCliPrintCurrentHelp(EmbeddedCli* cli);

/**
 * @brief Enter Sub-Interpreter mode
 * @param  cli
 * @param  onCommand
 * @param  onExit called when Ctrl+D or manual exit is called
 * @param  invitation
 */
void embeddedCliEnterSubInterpreter(
    EmbeddedCli* cli, void (*onCommand)(EmbeddedCli* cli, CliCommand* command),
    void (*onExit)(EmbeddedCli* cli), const char* invitation);

/**
 * @brief Exit Sub-Interpreter mode
 */
void embeddedCliExitSubInterpreter(EmbeddedCli* cli);

/**
 * @brief Set invitation for cli
 * @param cli
 * @param invitation
 */
void embeddedCliSetInvitation(EmbeddedCli* cli, const char* invitation);

/**
 * @brief Set Raw-Hanlder for cli
 * @param cli
 * @param rawHandler called when any raw data is received, return data to echo
 */
void embeddedCliSetRawHandler(EmbeddedCli* cli,
                              char (*rawHandler)(EmbeddedCli* cli, char data));

/**
 * @brief Reset Raw-Handler for cli
 */
void embeddedCliResetRawHandler(EmbeddedCli* cli);

/**
 * @brief Set Raw-Buffer-Handler for cli
 * @param cli
 * @param rawBufferHandler called when any raw data is received
 */
void embeddedCliSetRawBufferHandler(EmbeddedCli* cli,
                                    void (*rawBufferHandler)(EmbeddedCli* cli,
                                                             const char* buffer,
                                                             size_t len));

/**
 * @brief Reset Raw-Buffer-Handler for cli
 * @param cli
 */
void embeddedCliResetRawBufferHandler(EmbeddedCli* cli);

/**
 * @brief Set onCommandExecution callback
 * @param cli
 * @param onCommandExecution called before and after command execution
 */
void embeddedCliSetOnCommandExecution(
    EmbeddedCli* cli,
    void (*onCommandExecution)(EmbeddedCli* cli, CliCommand* command,
                               bool is_finished));

/**
 * @brief Reset onCommandExecution callback
 * @param cli
 */
void embeddedCliResetOnCommandExecution(EmbeddedCli* cli);

#ifdef __cplusplus
}
#endif

#endif /* __EMBEDDED_CLI_H__ */
