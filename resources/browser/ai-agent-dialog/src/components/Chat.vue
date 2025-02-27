<template>
  <!-- demo/textInput are examples of passing an object directly into a property (if this does not work refer to data values like in the history example) -->
  <!-- history is an example of passing a data object into a property -->
  <deep-chat
    :demo="true"
    :textInput="{ placeholder: { text: 'Welcome to the demo!' } }"
    :history="history"
    :request="{
      url: 'https://open.bigmodel.cn/api/paas/v4/chat/completions'
    }"
  />
</template>

<script>
import 'deep-chat';

export default {
  name: 'App',
  data() {
    return {
      history: [
        { role: 'user', text: 'Hey, how are you today?' },
        { role: 'ai', text: 'I am doing very well!' },
      ],
    };
  },
};
</script>

<style>
div {
  font-family: sans-serif;
  text-align: center;
  justify-content: center;
  display: grid;
}
</style>
