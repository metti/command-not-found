function __fish_command_not_found_handler --on-event fish_command_not_found
	if test -x "/usr/bin/cnf-lookup"
		cnf-lookup -c -- $argv[1]
		if test $status -ne 0
			__fish_default_command_not_found_handler $argv[1]
		end
	else
		__fish_default_command_not_found_handler $argv[1]
	end
	# return 127 # This is actually unnecessary; fish handles it
end

