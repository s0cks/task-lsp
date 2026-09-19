package lsp

import (
	"taskfile-lsp/internal/document"
	"taskfile-lsp/internal/workspace"
)

type Document struct {
	URI         string
	LanguageID  string
	Version     int
	Text        string
	Diagnostics []Diagnostic

	parsed *document.Document
}

func (d *Document) Access(fn func(doc *document.Document, text string)) {
	fn(d.parsed, d.Text)
}

type DocumentStore struct {
	ws          *workspace.Workspace
	languageIDs map[string]string
}

func NewDocumentStore(ws *workspace.Workspace) *DocumentStore {
	return &DocumentStore{ws: ws, languageIDs: make(map[string]string)}
}

func (s *DocumentStore) snapshot(uri string) (*Document, bool) {
	f, ok := s.ws.File(uri)
	if !ok {
		return nil, false
	}

	return &Document{
		URI:        uri,
		LanguageID: s.languageIDs[uri],
		Version:    f.Version,
		Text:       f.Text,
		parsed:     f.Parsed,
	}, true
}

func (s *DocumentStore) Open(doc *Document) []string {
	s.languageIDs[doc.URI] = doc.LanguageID
	return s.ws.Load(doc.URI, doc.Version, doc.Text, true)
}

func (s *DocumentStore) Get(uri string) (*Document, bool) {
	return s.snapshot(uri)
}

func (s *DocumentStore) Update(uri string, version int, text string) []string {
	return s.ws.Load(uri, version, text, true)
}

func (s *DocumentStore) Close(uri string) {
	s.ws.Close(uri)
	delete(s.languageIDs, uri)
}
