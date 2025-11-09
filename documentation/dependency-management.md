# CMake Dependency Management

This document explains how CMake dependencies are managed in this project, since Dependabot does not support the CMake ecosystem.

## Current Dependencies

### TeDDy (DecisionDiagrams) Library
- **Repository**: https://github.com/MichalMrena/DecisionDiagrams
- **Current Version**: v4.1.0 (see `CMakeLists.txt`)
- **Fetch Method**: CMake FetchContent with Git
- **Purpose**: Binary Decision Diagram (BDD) implementation

## Automated Dependency Monitoring

### GitHub Actions Workflow
This project uses a custom GitHub Actions workflow (`.github/workflows/dependency-check.yml`) to automatically check for updates:

- **Schedule**: Runs weekly on Mondays at 06:00 UTC
- **Manual Trigger**: Can be triggered manually from the Actions tab
- **Process**:
  1. Checks the current TeDDy version in `CMakeLists.txt`
  2. Queries GitHub API for the latest release
  3. Compares versions and creates an issue if an update is available
  4. Optionally creates a pull request with the update (manual trigger only)

### Viewing Dependency Status
1. Go to the **Actions** tab in GitHub
2. Select **CMake Dependency Check** workflow
3. View recent workflow runs to see if updates are available

## Manual Update Process

### Updating TeDDy Library

1. **Check for new releases**:
   - Visit https://github.com/MichalMrena/DecisionDiagrams/releases
   - Review the release notes for breaking changes

2. **Update CMakeLists.txt**:
   ```cmake
   FetchContent_Declare(
       DecisionDiagrams
       GIT_REPOSITORY https://github.com/MichalMrena/DecisionDiagrams.git
       GIT_TAG v4.2.0  # Update this line
   )
   ```

3. **Clean build directory** (recommended for major updates):
   ```bash
   rm -rf build
   cmake -B build -S .
   ```

4. **Build and test**:
   ```bash
   cmake --build build --config Release
   ctest --test-dir build --output-on-failure
   ```

5. **Verify all tests pass** before committing the change

6. **Commit with conventional commit message**:
   ```bash
   git add CMakeLists.txt
   git commit -s -m "chore(deps): Update TeDDy library from v4.1.0 to v4.2.0"
   ```

## Alternative Monitoring Methods

### 1. GitHub Watch + Notifications
- Navigate to https://github.com/MichalMrena/DecisionDiagrams
- Click **Watch** → **Custom** → Select **Releases**
- You'll receive notifications when new versions are released

### 2. RSS Feed
Subscribe to the releases feed:
```
https://github.com/MichalMrena/DecisionDiagrams/releases.atom
```

### 3. GitHub CLI
Check for updates manually:
```bash
gh release view --repo MichalMrena/DecisionDiagrams --json tagName
```

### 4. Renovate Bot
Consider using [Renovate](https://github.com/renovatebot/renovate) as an alternative to Dependabot:
- Supports CMake via FetchContent
- Can automatically create PRs for dependency updates
- More flexible configuration than Dependabot

To enable Renovate:
1. Install the Renovate GitHub App
2. Create `.github/renovate.json`:
   ```json
   {
     "$schema": "https://docs.renovatebot.com/renovate-schema.json",
     "extends": ["config:base"],
     "packageRules": [
       {
         "matchManagers": ["cmake"],
         "enabled": true
       }
     ]
   }
   ```

## Security Updates

### CVE Monitoring
While Dependabot can't scan CMake dependencies, you can:

1. **Subscribe to security advisories**:
   - Watch the DecisionDiagrams repository for security alerts
   - Monitor https://github.com/MichalMrena/DecisionDiagrams/security/advisories

2. **Use GitHub Security Alerts** (if enabled):
   - Check the **Security** tab in this repository
   - GitHub may detect vulnerabilities in fetched dependencies

3. **Manual vulnerability scanning**:
   - Periodically review the dependency's security status
   - Check for CVEs affecting the specific version in use

## Best Practices

1. **Regular Updates**: Review and update dependencies quarterly, even without security issues
2. **Test Thoroughly**: Always run the full test suite after updating
3. **Version Pinning**: Use specific version tags (e.g., `v4.1.0`) instead of branches (e.g., `main`)
4. **Document Breaking Changes**: Update this file when major version updates occur
5. **Monitor CI/CD**: The automated workflow will alert you to available updates

## Troubleshooting

### Build Failures After Update
1. Clear CMake cache: `rm -rf build/CMakeCache.txt build/CMakeFiles`
2. Rebuild from scratch: `rm -rf build && cmake -B build`
3. Check TeDDy release notes for API changes
4. Review error messages for deprecated functions

### Version Mismatch
If you see unexpected behavior:
1. Verify the version in `CMakeLists.txt` matches what CMake fetched
2. Check `build/_deps/decisiondiagrams-src/` for the actual source
3. Clean and rebuild to ensure correct version is fetched

## Future Improvements

Potential enhancements to dependency management:
- [ ] Add more CMake dependencies and track them in the workflow
- [ ] Integrate with Renovate Bot for automated PRs
- [ ] Add dependency version matrix testing (test against multiple TeDDy versions)
- [ ] Create automated changelogs when dependencies are updated

## Questions or Issues?

If you encounter problems with dependency management:
1. Check existing issues: https://github.com/Alan-Jowett/bdd_test/issues
2. Create a new issue with the `dependencies` label
3. Tag @Alan-Jowett for assistance
