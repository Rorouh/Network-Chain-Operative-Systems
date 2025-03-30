SOchain Project - Git Usage Guide

This guide outlines the minimal steps to use Git for the SOchain project.
1. Clone the Repository

Clone the remote repository to your local machine:

git clone https://github.com/alejandroldz/SO.git

This command will create a folder named SO containing the project files.
2. Create a New Branch (Optional)

If you want to work on a new feature or fix, create and switch to a new branch:

git checkout -b my-feature-branch

3. Make Changes and Stage Files

Edit or add new files in your project. When you're ready to save your changes, stage the files:

git add .

This stages all modified and new files.
4. Commit Your Changes

Commit your changes with a descriptive message:

git commit -m "Describe your changes here"

5. Push Changes to the Remote Repository

Push your changes to the remote repository. If your main branch is called main, use:

git push origin main

If you're working on a feature branch, push it with:

git push origin my-feature-branch

6. Pull Changes from Remote

Before starting new work, update your local repository:

git pull origin main

Replace main with your branch name if needed.
7. Additional Tips

    Use git status to check the current state of your repository.

    Use git log to view the commit history.

    Write clear commit messages to help your team understand your changes.

    If you encounter merge conflicts during a pull, resolve them manually and then commit the changes.
