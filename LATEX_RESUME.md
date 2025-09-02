# LaTeX Resume Build System

This repository includes an automated LaTeX resume compilation system that builds your resume to PDF and publishes it to GitHub releases.

## Files

- `resume.tex` - Your LaTeX resume source file
- `.github/workflows/latex-resume.yml` - GitHub Action workflow for automated compilation

## How it works

The GitHub Action workflow will automatically:

1. **Trigger** when you:
   - Push changes to `resume.tex` on the main/master branch
   - Push changes to the workflow file
   - Manually trigger the workflow
   - Create a new release

2. **Compile** your LaTeX resume using:
   - XeLaTeX engine for better font support
   - Full LaTeX scheme with all packages
   - Modern CV class for professional formatting

3. **Publish** the resulting PDF by:
   - Creating a GitHub release with a date-stamped tag
   - Uploading the compiled PDF as a release asset
   - Including build metadata in the release notes

## Usage

### Editing your resume

1. Edit the `resume.tex` file with your personal information
2. Customize sections like education, experience, projects, and skills
3. Commit and push your changes to the main branch

### Manual compilation

The workflow will automatically trigger, but you can also:
- Go to the "Actions" tab in GitHub
- Select "Build and Release LaTeX Resume" 
- Click "Run workflow" to manually trigger compilation

### Accessing your PDF

After successful compilation:
1. Go to the "Releases" section of your repository
2. Download the latest PDF from the most recent release
3. Or check the "Actions" tab for the workflow artifacts

## Customization

### Resume template

The default `resume.tex` uses the `moderncv` class. You can:
- Change the style: `\moderncvstyle{classic|casual|oldstyle|banking}`
- Change colors: `\moderncvcolor{blue|orange|green|red|purple|grey|black}`
- Modify sections and content as needed

### Workflow configuration

Edit `.github/workflows/latex-resume.yml` to:
- Change trigger conditions
- Modify LaTeX compilation options
- Customize release naming or description

## Requirements

- Your repository must have Actions enabled
- The `resume.tex` file must be valid LaTeX
- Uses the `moderncv` LaTeX class (included in full scheme)

## Troubleshooting

If compilation fails:
1. Check the Actions logs for LaTeX errors
2. Ensure your `resume.tex` syntax is valid
3. Make sure all required packages are available (full scheme includes most)
4. Test locally if possible with XeLaTeX

The workflow uses `xu-cheng/latex-action@v3` which provides a comprehensive LaTeX environment with most common packages pre-installed.