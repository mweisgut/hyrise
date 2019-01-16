#!/bin/bash

set -e

if [[ $# -lt 3 ]]
then
	echo Runs a benchmark for every commit in parameters
	echo Call the script from your build directory \(i.e., ../scripts/$0\)
	echo Arguments: commitid_file binary [benchmark_arguments]
	echo binary is, for example, hyriseMicroBenchmarks
	exit 1
fi

commit_list=$1
benchmark=$2
shift; shift
benchmark_arguments=$@
initial_branch=$(git rev-parse --abbrev-ref HEAD)

if [[ $(git status --untracked-files=no --porcelain) ]]
then
	echo Cowardly refusing to execute on a dirty workspace
	exit 1
fi

for commit in $(cat ${commit_list})
do
	if [[ -f auto_${commit}.json ]]; then
		continue
	fi

	echo =======================================================
	git checkout ${commit}

    echo =======================================================
    echo Building Binary

	cores=$(grep -c ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu)
	make ${benchmark} -j $((cores / 2)) > /dev/null

    echo =======================================================
    echo Running Benchmark

	./${benchmark} ${benchmark_arguments} --benchmark_out=auto_${commit}.json --benchmark_out_format=json
done

echo =======================================================
echo Switching back to initial Branch

git checkout ${initial_branch}