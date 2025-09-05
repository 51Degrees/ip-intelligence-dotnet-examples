# API Specific CI Approach

For the general CI Approach, see [common-ci](https://github.com/51degrees/common-ci).

The following secrets are required:
* `ACCESS_TOKEN` - GitHub [access token](https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/managing-your-personal-access-tokens#about-personal-access-tokens) for cloning repos, creating PRs, etc.
    * Example: `github_pat_l0ng_r4nd0m_s7r1ng`

### Differences
- There are no packages produced by this repository, so the only output from the `Nightly Publish Main` workflow is a new tag and release.
- The package update step does not update dependencies from a package manager in the same way as other repos.

### Build Options

For additional build options in this repo see [common-ci/cxx](https://github.com/51Degrees/common-ci/tree/main/cxx#readme)


## Prerequisites

Only the [common prerequisites](https://github.com/51Degrees/common-ci#prerequisites) are required for this repository.
