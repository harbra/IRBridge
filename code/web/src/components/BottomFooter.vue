<!--
This vue.js file contains a footer for the IRBridge project.
-->
<template>
  <footer class="footer">
    <div class="container">
      <div class="content has-text-centered">
        <p>
          <strong>IRBridge</strong> created with <i class="icon-heart"></i> by <a href="mailto:me@haraldbraun.name">Harald Braun</a>. This website is served from the ESP8266 <code>#{{ serverinfo.chipid.toString(16) }}</code> under <code>{{ serverinfo.ip }}</code>.
        </p>
      </div>
    </div>
  </footer>
</template>

<script>
import {HTTP} from '../plugins/axios'

export default {
  name: 'bottom-footer',
  data () {
    return {
      serverinfo: {ip: '[unknown]',
                   chipid: '[unknown]'}
    }
  },
    created: function() {
      /*
      When the component is created, try to get the ESP's IP and Chip-ID.
      If that fails, reset the values to '[unknown]'
      */
      HTTP.get('/api/getinfo').then((response) =>
        {
          if(response.status==200 && (response.headers["content-type"].includes("application/json") || response.headers["content-type"].includes("text/json")))
          {
            this.serverinfo=response.data;
          } else {
            this.serverinfo={ip: '[unknown]',
                         chipid: '[unknown]'};
          }
        }).catch((e) =>
        {
          this.serverinfo={ip: '[unknown]',
                       chipid: '[unknown]'};
        })
  }
}
</script>
