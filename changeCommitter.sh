git filter-branch --commit-filter '
        if [ "$GIT_COMMITTER_NAME" = "志水 友輔" ];
        then
                GIT_COMMITTER_NAME="Yusuke-Shimizu";
                GIT_AUTHOR_NAME="Yusuke-Shimizu";
                GIT_COMMITTER_EMAIL="shimi.nekoze@gmail.com";
                GIT_AUTHOR_EMAIL="shimi.nekoze@gmail.com";
                git commit-tree "$@";
        else
                git commit-tree "$@";
        fi' HEAD~10..HEAD
