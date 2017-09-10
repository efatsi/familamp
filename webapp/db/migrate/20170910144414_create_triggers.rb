class CreateTriggers < ActiveRecord::Migration[5.0]
  def change
    create_table :triggers do |t|
      t.string :color
      t.integer :family_member_id

      t.timestamps
    end
  end
end
