package lsp

import (
	"log"
	"sync"

	"taskfile-lsp/internal/taskfile"
)

type Document struct {
	URI        string
	LanguageID string
	Version    int
	Text       string
	Parsed     *taskfile.File
}

type DocumentStore struct {
	log  *log.Logger
	mu   sync.RWMutex
	docs map[string]*Document
}

func NewDocumentStore(log *log.Logger) *DocumentStore {
	return &DocumentStore{log: log, docs: make(map[string]*Document)}
}

func (s *DocumentStore) Open(doc *Document) []taskfile.Diagnostic {
	parser := taskfile.Parser{Log: s.log}
	parsed, diags := parser.Parse(doc.Text)
	doc.Parsed = parsed
	s.mu.Lock()
	defer s.mu.Unlock()
	s.docs[doc.URI] = doc
	return diags
}

func (s *DocumentStore) Close(uri string) {
	s.mu.Lock()
	defer s.mu.Unlock()
	delete(s.docs, uri)
}

func (s *DocumentStore) Get(uri string) (*Document, bool) {
	s.mu.RLock()
	defer s.mu.RUnlock()
	d, ok := s.docs[uri]
	return d, ok
}

func (s *DocumentStore) Update(uri string, version int, text string) []taskfile.Diagnostic {
	parser := taskfile.Parser{Log: s.log}
	parsed, diags := parser.Parse(text)
	s.mu.Lock()
	defer s.mu.Unlock()
	if d, ok := s.docs[uri]; ok {
		d.Version = version
		d.Text = text
		d.Parsed = parsed
	}

	return diags
}
