
var switchUIClass = {
  // Model for the switch module
  Model: Backbone.Model.extend({
    defaults: function () {
      let model = {
        id: "",
        status: ""  // "on" or "off"
        
      };
      return model;
    }

  }),

  // View for the switch module
  View: Backbone.View.extend({
    tagName: "div",
    template: _.template('<div>Switch is <%- status %></div>'),
    initialize: function () {
      this.listenTo(this.model, 'change', this.render);
    },
    render: function () {
      this.$el.html(this.template(this.model.toJSON()));
      return this;
    }
  })

  
};