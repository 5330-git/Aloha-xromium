import { fileURLToPath, URL } from 'node:url'

import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import vueDevTools from 'vite-plugin-vue-devtools'
import legacy from '@vitejs/plugin-legacy';

// https://vite.dev/config/
export default defineConfig({
  plugins: [
    vue(),
    vueDevTools(),
    legacy({
      targets: ['defaults', 'not IE 11']
    }),
  ],
  base: "./",
  resolve: {
    alias: {
      '@': fileURLToPath(new URL('./src', import.meta.url))
    },
  },
  build: {
    rollupOptions: {
      output: {
        // 固定入口 chunk 文件名（JS）
        entryFileNames: 'ai-agent-dialog-[name].js',
        // 固定非入口 chunk 文件名（JS）
        chunkFileNames: 'ai-agent-dialog-[name]-chunk.js',
        // 固定 CSS 文件名
        assetFileNames: 'ai-agent-dialog-[name].css'
      }
    }
  },
  server: {
    fs: {
      // 允许访问项目根目录下的文件
      allow: ['../']
    }
  }
})
