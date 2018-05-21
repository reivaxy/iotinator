
var switchUIClass = {
  // Model for the switch module
  Model: Backbone.Model.extend({
    defaults: function () {
      let model = {
        id: "",
        status: ""  // "on" or "off"
        
      };
      return model;
    },
    
    set: function(data) {
      // If some pre processing is needed it can be done here (instead of in parse)
      if(data.status == "on") {
        data.checkedOn = "checked";
        data.activeOn = "active";
        data.checkedOff = "";
        data.activeOff = "";
      } else {
        data.checkedOff = "checked";
        data.activeOff = "active";
        data.checkedOn = "";
        data.activeOn = "";

      }
      Backbone.Model.prototype.set.call(this, data);
    }

  }),

  // View for the switch module
  View: Backbone.View.extend({
    tagName: "div",
    template: _.template('\
<!--<div>Switch is <%- status %></div>-->\
<div class="btn-group btn-group-toggle" data-toggle="buttons">\
  <label class="btn btn-secondary <%- activeOn %>">\
    <input type="radio" name="options" id="optionOn" autocomplete="off" <%- checkedOn %>> On\
  </label>\
  <label class="btn btn-secondary <%- activeOff %>">\
    <input type="radio" name="options" id="optionOff" autocomplete="off" <%- checkedOff %>> Off\
  </label>\
</div>'),
    
    initialize: function () {
      this.listenTo(this.model, 'change', this.render);
    },
    events: {
      "click label.btn": "toggle",
    },
    toggle: function(e) {
      if(this.model.attributes.checkedOn == "") {
        this.model.attributes.checkedOn = "checked";
        this.model.attributes.activeOn = "active";
        this.model.attributes.checkedOff = "";
        this.model.attributes.activeOff = "";
        
      } else {
        this.model.attributes.checkedOff = "checked";
        this.model.attributes.activeOff = "active";
        this.model.attributes.checkedOn = "";
        this.model.attributes.activeOn = "";

      }
      this.render();
    },
    render: function () {
      this.$el.html(this.template(this.model.toJSON()));
      return this;
    }
  })

  
};