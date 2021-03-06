# zsh
if [ -n "${ZSH_NAME}" ]; then
    command_not_found_handler () {
        if [ -x /usr/bin/cnf-lookup ]; then
            cnf-lookup -c -- $1
            if [ ! $? -eq 0 ]; then
                echo "zsh: $1: command not found"
            fi
        else
            echo "zsh: $1: command not found"
        fi
        return 127
    }
fi

# bash
if [ -n "${BASH}" ]; then
    command_not_found_handle () {
        if [ -x /usr/bin/cnf-lookup ]; then
            cnf-lookup -c -- $1
            if [ ! $? -eq 0 ]; then
                echo "bash: $1: command not found"
            fi
        else
            echo "bash: $1: command not found"
        fi
        return 127
    }
fi

