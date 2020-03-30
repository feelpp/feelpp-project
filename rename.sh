#! /bin/bash

in_default_projectname="project"
read -p "What is the name of the project ? [$in_default_projectname]: " projectname
projectname=${projectname:-$in_default_projectname}

in_default_titlename="${projectname^}"
read -p "What is the title of the project? [$in_default_titlename]" projecttitle
projecttitle=${projecttitle:-$in_default_titlename}

in_default_shortname=""
read -p "What is the short name of the project? []" shortprojectname
shortprojectname=${shortprojectname:-$in_default_shortname}

in_default_appname="myapp"
read -p "What is the name of the first application? [$in_default_myapp]" appname
appname=${appname:-$in_default_appname}

echo "     project name: $projectname"
echo "    project title: $projecttitle"
echo "project shortname: $shortprojectname" 
echo "         app name: $appname"
echo "Do you wish to rename this project?"
select yn in "Yes" "No"; do
    case $yn in
        Yes ) echo "Ok lets go!"; break;;
        No ) exit;;
    esac
done

export projectname
export projecttitle
export shortprojectname
export appname

perl -077pi.bak -e 's/myproject/$ENV{'projectname'}/sg' CMakeLists.txt
if test -z "$shortprojectname"; then
    perl -077pi.bak -e 's/set\\(PROJECT_SHORTNAME \"\"\\)/set(PROJECT_SHORTNAME "$ENV{'shortprojectname'}")/sg' CMakeLists.txt
fi    
perl -077pi.bak -e 's/myapp/$ENV{'appname'}/sg' $PWD/src/CMakeLists.txt

perl -077pi.bak -e 's/myproject/$ENV{'projectname'}/sg' README.adoc

git mv src/.tests.myapp  src/.tests.$appname

