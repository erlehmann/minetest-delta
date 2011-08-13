#!/bin/sh

# Update/create minetest-delta delta branch

# Options
verbose_opt=

verbose() {
	test -n "$verbose_opt"
}

update_remotes_opt=

update_remotes() {
	test -n "$update_remotes_opt"
}

# an auxiliary function to abort processing with an optional error
# message
abort() {
	test -n "$1" && echo >&2 "$1"
	exit 1
}

# an auxiliary function stolen from git-sh-setup
require_clean_work_tree () {
	git rev-parse --verify HEAD >/dev/null || exit 1
	git update-index -q --ignore-submodules --refresh
	err=0

	if ! git diff-files --quiet --ignore-submodules
	then
		echo >&2 "Cannot $1: You have unstaged changes."
		err=1
	fi

	if ! git diff-index --cached --quiet --ignore-submodules HEAD --
	then
		if [ $err = 0 ]
		then
		    echo >&2 "Cannot $1: Your index contains uncommitted changes."
		else
		    echo >&2 "Additionally, your index contains uncommitted changes."
		fi
		err=1
	fi

	if [ $err = 1 ]
	then
		test -n "$2" && echo >&2 "$2"
		exit 1
	fi
}


# an auxiliary function to test if a given remote exists already
remote_exists() {
	remote="$1"
	test -n "$(git config --get "remote.$remote.url")"
}

# an auxiliary function to test if a given remote has a given
# (fetch) url
remote_url_match() {
	remote="$1"
	url="$2"
	test "$(git config --get "remote.$remote.url")" = "$url"
}

# an auxiliary function to mark and echo a reflog action
mark_reflog() {
	reason="$1"
	test -n "$1" || abort "reflog mark without action"
	GIT_REFLOG_ACTION="$1"
	export GIT_REFLOG_ACTION
	echo "Step: $1"
}

unmark_reflog() {
	unset GIT_REFLOG_ACTION
}

# First things first: check that we have a clean work-tree
require_clean_work_tree "rebuild delta"

# We look for mergeable forks/branches in a file called delta-branches
# located in the same directory as the script.
scriptisin="$(dirname "$(which "$0")")"

branchfile="$scriptisin/delta-branches"

test -e "$branchfile" || abort "Branch file '$branchfile' not found!"
test -r "$branchfile" || abort "Branch file '$branchfile' cannot be read!"

# The branch file has a rather simple syntax, with three whitespace-separated
# fields: a remote name, a remote url and a branch name.

# FIXME: forcing a remote name is probably not very smart: it can conflict
# with user-chosen names for other remotes. Think about a better approach

forks=

{
	# The first remote fork _must_ be celeron55's master
	celeron55checked=

	while read remote url branch junk ; do
		comment=
		if verbose ; then
			test -z "$junk" || comment=" ($junk)"
			echo "Remote '$remote' with url '$url': branch '$branch'$comment"
		fi

		if test -z "$celeron55checked" ; then
			test	"$remote" = celeron55 -a \
				"$url" = git://github.com/celeron55/minetest.git -a \
				"$branch" = master || abort "First remote is not celeron55's master!"
			celeron55checked=t
		fi

		if remote_exists "$remote" ; then
			if remote_url_match "$remote" "$url" ; then
				verbose && echo "Remote '$remote' with url '$url' exists already, good"
			else
				abort "Remote '$remote' exists, but its url is not '$url', please fix this"
			fi
		else
			echo "Creating remote '$remote' with url '$url'"
			git remote add "$remote" "$url" || abort "Failed to create remote"
		fi

		fork="$remote/$branch"
		# Don't add celeron55/master to the delta forks: master should take care of it
		test "$fork" = "celeron55/master" || forks="$forks $fork"
	done
} < "$branchfile"

if update_remotes ; then
	echo "Updating remotes"
	git remote update
fi

missing="$(git rev-list master..celeron55/master | wc -l)"

if test "$missing" -gt 0 ; then
	echo "WARNING: your master is behind celeron55's by $missing commits!"
fi

mark_reflog "reset delta branch"
git branch --no-track -l -f delta master

mark_reflog "switch to delta branch"
git checkout delta

for fork in $forks; do
	mark_reflog "merge $fork"
	git merge "$fork" || abort "Failed!"
	echo "Build-testing"
	make || abort "Failed!"
done

unmark_reflog



