<script lang="ts">
  import { onMount } from "svelte";
  const docModules = import.meta.glob("./docs/*.svx");

  let docList = $state<{ slug: string; title: string; section: string }[]>([]);
  let currentSlug = $state("introduction");
  let ActiveComponent = $state<any>(null);

  onMount(async () => {
    const structuralList = [];
    for (const path in docModules) {
      const slug = path.split("/").pop()?.replace(".svx", "") || "";
      const module: any = await docModules[path]();

      structuralList.push({
        slug,
        title: module.metadata?.title || slug,
        section: module.metadata?.section || "General",
      });
    }
    docList = structuralList;
    loadDoc(currentSlug);
  });

  async function loadDoc(slug: string) {
    currentSlug = slug;
    const targetPath = `./docs/${slug}.svx`;

    if (docModules[targetPath]) {
      const module: any = await docModules[targetPath]();
      // This safely injects the compiled Svelte component into your UI layout
      ActiveComponent = module.default;
    } else {
      console.error(`Dynamic compilation map failed to locate component at: ${targetPath}`);
    }
  }
</script>

<div class="flex min-h-screen font-sans bg-gray-50 text-gray-900">
  <!-- Custom Sidebar Menu -->
  <aside class="w-64 bg-white border-r border-gray-200 p-6">
    <h2 class="text-xl font-bold mb-6 text-indigo-600">Documentation</h2>
    <nav class="space-y-1">
      {#each docList as doc}
        <button
          onclick={() => loadDoc(doc.slug)}
          class="w-full text-left px-3 py-2 rounded-lg text-sm font-medium transition-colors
                           {currentSlug === doc.slug
            ? 'bg-indigo-50 text-indigo-700'
            : 'text-gray-600 hover:bg-gray-100'}"
        >
          {doc.title}
        </button>
      {/each}
    </nav>
  </aside>

  <!-- Main Content Display -->
  <main class="flex-1 max-w-4xl px-12 py-10">
    <article class="prose max-w-none bg-white p-8 rounded-xl shadow-sm border border-gray-100">
      {#if ActiveComponent}
        <ActiveComponent />
      {:else}
        <p class="text-gray-400 animate-pulse">Loading template mount points...</p>
      {/if}
    </article>
  </main>
</div>
