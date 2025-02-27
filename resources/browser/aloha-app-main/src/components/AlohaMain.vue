<template>
  <div class="search-page">
    <el-container>
      <el-main style="height: 100vh;">
        <el-row justify="center" :gutter="20">
          <el-col :xs="10" :sm="9" :md="7" :lg="4" :push="2">
            <div>
              <img src="/chromium_logo.png" style="width: 100px; height: 100px;">
            </div>
          </el-col>
          <el-col :xs="8" :sm="5" :md="5" :lg="9" :pull="2">
            <h1 style="text-align: center;">Welcome Aloha</h1>
          </el-col>
        </el-row>
        <br>
        <br>
        <el-row justify="center">
          <el-col :xs="22" :sm="20" :md="14" :lg="10" :pull="1">
            <div>

              <el-select v-model="title" clear-icon filterable remote reserve-keyword
                placeholder="Please enter a keyword" :remote-method="remoteMethod" :loading="loading" :width="auto"
                @change="handleSelected">
                <el-option v-for=" item in options" :key="item.id" :label="item.label" :value="item.value" />
              </el-select>
            </div>
          </el-col>
        </el-row>
      </el-main>
    </el-container>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue';
const options = ref([]);
const title = ref([]);
const list = ref([]);
const dict = ref({});
const loading = ref(false);
onMounted(() => {
  list.value = [
    { id: 1, value: 'Ai Dialog', url: 'file:///D:/codes/build-chromium/chromium/src/aloha/resources/browser/ai-agent-dialog/dist/index.html' },
    { id: 2, value: 'Bilibili', url: 'https://www.bilibili.com' },
    { id: 3, value: 'Vue Router Documentation', url: 'https://router.vuejs.org/' },
    { id: 4, value: 'dev', url: 'http://localhost:5173/' }
  ]
  list.value.forEach(item => {
    dict.value[item.value] = item.url
  })
})

const remoteMethod = (query) => {
  if (query) {
    console.log(query);
    console.log(list.value);
    loading.value = true
    setTimeout(() => {
      loading.value = false
      options.value = list.value.filter((item) => {
        return item.value.toLowerCase().includes(query.toLowerCase())
      })
    }, 200)
  } else {
    options.value = []
  }
}
const handleSelected = () => {
  title.value = title.value.trim()
  console.log(dict.value[title.value]);
  try {
    let url = new URL(dict.value[title.value])
    if (url.protocol === 'file:') {
      // 在当前页面打开
      window.location.href = url.href
    } else {
      // 在新标签页打开
      window.open(url.href, '_blank').focus()
    }
  }
  catch (e) {
    console.log(e);
  }

}
</script>

<style scoped>
.search-page {
  padding: 20px;
}

/* h1 {
  text-align: center;
} */
</style>