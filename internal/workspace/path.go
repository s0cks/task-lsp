package workspace

import (
	"fmt"
	"net/url"
	"os"
	"path/filepath"

	"taskfile-lsp/internal/document"
)

func URIToPath(uri string) (string, error) {
	u, err := url.Parse(uri)
	if err != nil {
		return "", err
	}

	if u.Scheme != "file" {
		return "", fmt.Errorf("unsupported URI scheme %q", u.Scheme)
	}

	return u.Path, nil
}

func PathToURI(path string) string {
	abs, err := filepath.Abs(path)
	if err != nil {
		abs = path
	}

	u := url.URL{Scheme: "file", Path: filepath.ToSlash(abs)}
	return u.String()
}

var defaultTaskfileNames = []string{"Taskfile.yml", "Taskfile.yaml", "taskfile.yml", "taskfile.yaml"}

func findDefaultTaskfile(dir string) (string, bool) {
	for _, name := range defaultTaskfileNames {
		p := filepath.Join(dir, name)
		if info, err := os.Stat(p); err == nil && !info.IsDir() {
			return p, true
		}
	}

	return "", false
}

func resolveIncludePath(includingFilePath string, inc document.Include) (string, error) {
	baseDir := filepath.Dir(includingFilePath)

	if d := inc.Dir(); d != "" {
		if filepath.IsAbs(d) {
			baseDir = d
		} else {
			baseDir = filepath.Join(baseDir, d)
		}
	}

	tf := inc.Taskfile()
	if tf == "" {
		if p, ok := findDefaultTaskfile(baseDir); ok {
			return p, nil
		}

		return "", fmt.Errorf("no default taskfile found in %s", baseDir)
	}

	target := tf
	if !filepath.IsAbs(target) {
		target = filepath.Join(baseDir, target)
	}

	if info, err := os.Stat(target); err == nil {
		if info.IsDir() {
			if p, ok := findDefaultTaskfile(target); ok {
				return p, nil
			}

			return "", fmt.Errorf("no default taskfile found in %s", target)
		}

		return target, nil
	}

	for _, ext := range []string{".yml", ".yaml"} {
		if info, err := os.Stat(target + ext); err == nil && !info.IsDir() {
			return target + ext, nil
		}
	}

	return "", fmt.Errorf("taskfile not found: %s", target)
}
